/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_shell.c
      生成日期: 2016.09.26
      作    者：like
      功能说明：shell任务模块
      其他说明：1. 调试口默认是开启打印调试信息
                2. 当连续接收到5个回车，打印信息功能关闭，开启shell命令行模式
                3. 当进入shell命令行模式，连续按1次esc键，退出shell命令行模式，
                   进入打印信息模式
                4. 当前为命令行模式后，5分钟没有收到数据，将推出shell命令行模式，
                   进入打印信息模式
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h> 

#include "bsp_uart.h"
#include "shell_cmd.h"


#define FINSH_HISTORY_LINES 5
#define FINSH_CMD_SIZE		80
#define RT_FINSH_ARG_MAX    10

#define FINSH_PROMPT		"shell>>"


typedef long (*syscall_func)();


struct finsh_syscall
{
	const char*		name;
	syscall_func func;
};


struct finsh_sysvar
{
	const char *name;
	char  type;	
	void  *var;	
};


typedef int (*cmd_function_t)(int argc, char **argv);


enum input_stat
{
	WAIT_NORMAL,
	WAIT_SPEC_KEY,
	WAIT_FUNC_KEY,
};


struct finsh_shell
{
	enum input_stat stat;
	char echo_mode:1;

	unsigned short current_history;
	unsigned short history_count;

	char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];
    
	char line[FINSH_CMD_SIZE];
	char line_position;
	char line_curpos;
};


struct finsh_syscall _syscall_table[] =
{
	{"hello", hello},
	{"version", 0},
};


struct finsh_syscall *_syscall_table_begin = &_syscall_table[0];
struct finsh_syscall *_syscall_table_end   = &_syscall_table[sizeof(_syscall_table) / sizeof(struct finsh_syscall)];

struct finsh_sysvar *_sysvar_table_begin  = NULL;
struct finsh_sysvar *_sysvar_table_end    = NULL;

struct finsh_shell arg;



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int shell_byte_read(char *byte)
{
    int len;
    
    len = bsp_uart_receive(UART_1, byte, 1);
    if (len == 1)
    {
        return 1;
    }
    
    return 0;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static char shell_handle_history(struct finsh_shell *shell)
{
#if defined(_WIN32)
    int i;
    kprintf("\r");

    for (i = 0; i <= 60; i++)
        putchar(' ');
    kprintf("\r");

#else
    kprintf("\033[2K\r");
#endif
    kprintf("%s%s", FINSH_PROMPT, shell->line);
    return 0;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int msh_help(int argc, char **argv)
{
    kprintf("RT-Thread shell commands:\n");
    {
        struct finsh_syscall *index;

        for (index = _syscall_table_begin;
                index < _syscall_table_end;
                index++)
        {
            if (strncmp(index->name, "__cmd_", 6) != 0) continue;

            kprintf("%s ", &index->name[6]);
        }
    }
    kprintf("\n");

    return 0;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;

    while ((*str != 0) && (*str2 != 0) && (*str == *str2))
    {
        str ++;
        str2 ++;
    }

    return (str - str1);
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void msh_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct finsh_syscall *index;

    min_length = 0;
    name_ptr = 0;

    if (*prefix == '\0')
    {
        msh_help(0, 0);
        return;
    }
    
    {
        for (index = _syscall_table_begin; index < _syscall_table_end; index++)
        {
            if (strncmp(index->name, "__cmd_", 6) != 0) continue;

            cmd_name = (const char *) &index->name[6];
            if (strncmp(prefix, cmd_name, strlen(prefix)) == 0)
            {
                if (min_length == 0)
                {
                    name_ptr = cmd_name;
                    min_length = strlen(name_ptr);
                }

                length = str_common(name_ptr, cmd_name);
                if (length < min_length)
                    min_length = length;

                kprintf("%s\n", cmd_name);
            }
        }
    }

    if (name_ptr != NULL)
    {
        strncpy(prefix, name_ptr, min_length);
    }

    return ;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void shell_auto_complete(char *prefix)
{
    kprintf("\n");

    msh_auto_complete(prefix);

    kprintf("%s%s", FINSH_PROMPT, prefix);
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static cmd_function_t msh_get_cmd(char *cmd, int size)
{
    struct finsh_syscall *index;
    cmd_function_t cmd_func = 0;

    for (index = _syscall_table_begin;
            index < _syscall_table_end;
            index++)
    {
        if (strncmp(&index->name[0], cmd, size) == 0 &&
                index->name[size] == '\0')
        {
            cmd_func = (cmd_function_t)index->func;
            break;
        }
    }

    return cmd_func;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int msh_split(char *cmd, unsigned long length, 
                    char *argv[RT_FINSH_ARG_MAX])
{
    char *ptr;
    unsigned long position;
    unsigned long argc;

    ptr = cmd;
    position = 0; argc = 0;

    while (position < length)
    {
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++; position ++;
        }
        if (position >= length) break;

        if (*ptr == '"')
        {
            ptr ++; position ++;
            argv[argc] = ptr; argc ++;

            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++; position ++;
                    }
                }
                ptr ++; position ++;
            }
            if (position >= length) break;

            *ptr = '\0'; ptr ++; position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++; position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int _msh_exec_cmd(char *cmd, unsigned long length, int *retp)
{
    int argc;
    int cmd0_size = 0;
    cmd_function_t cmd_func;
    char *argv[RT_FINSH_ARG_MAX];

    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && cmd0_size < length)
        cmd0_size ++;
    if (cmd0_size == 0)
        return -1;

    cmd_func = msh_get_cmd(cmd, cmd0_size);
    if (cmd_func == 0)
        return -1;

    memset(argv, 0x00, sizeof(argv));
    argc = msh_split(cmd, length, argv);
    if (argc == 0)
        return -1;

    *retp = cmd_func(argc, argv);
    return 0;
}


int msh_exec(char *cmd, unsigned long length)
{
    int cmd_ret;

    while (*cmd  == ' ' || *cmd == '\t')
    {
        cmd++;
        length--;
    }

    if (length == 0)
        return 0;

    if (_msh_exec_cmd(cmd, length, &cmd_ret) == 0)
    {
        return cmd_ret;
    }

    {
        char *tcmd;
        tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0')
        {
            tcmd++;
        }
        *tcmd = '\0';
    }
    kprintf("%s: command not found.\n", cmd);
    
    return -1;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void shell_push_history(struct finsh_shell *shell)
{
    if (shell->line_position != 0)
    {
        if (shell->history_count >= FINSH_HISTORY_LINES)
        {
            int index;
            for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
            {
                memcpy(&shell->cmd_history[index][0],
                       &shell->cmd_history[index + 1][0], FINSH_CMD_SIZE);
            }
            memset(&shell->cmd_history[index][0], 0, FINSH_CMD_SIZE);
            memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

            shell->history_count = FINSH_HISTORY_LINES;
        }
        else
        {
            memset(&shell->cmd_history[shell->history_count][0], 0, FINSH_CMD_SIZE);
            memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

            shell->history_count ++;
        }
    }
    shell->current_history = shell->history_count;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void shell_init(void)
{
    uart_init(UART_1, 115200);  
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_shell(void *pvParameters)
{
    char ch;
    struct finsh_shell *shell = &arg;
    
    shell_init();
    shell->echo_mode = 1;
    kprintf(FINSH_PROMPT);
    
    while (1)
    {
        while (shell_byte_read(&ch) == 1)
        {
            if (ch == 0x1b)
            {
                shell->stat = WAIT_SPEC_KEY;
                continue;
            }
            else if (shell->stat == WAIT_SPEC_KEY)
            {
                if (ch == 0x5b)
                {
                    shell->stat = WAIT_FUNC_KEY;
                    continue;
                }

                shell->stat = WAIT_NORMAL;
            }
            else if (shell->stat == WAIT_FUNC_KEY)
            {
                shell->stat = WAIT_NORMAL;

                if (ch == 0x41)
                {
                    if (shell->current_history > 0)
                        shell->current_history --;
                    else
                    {
                        shell->current_history = 0;
                        continue;
                    }

                    memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                           FINSH_CMD_SIZE);
                    shell->line_curpos = shell->line_position = strlen(shell->line);
                    shell_handle_history(shell);
                    continue;
                }
                else if (ch == 0x42)
                {
                    if (shell->current_history < shell->history_count - 1)
                        shell->current_history ++;
                    else
                    {
                        if (shell->history_count != 0)
                            shell->current_history = shell->history_count - 1;
                        else
                            continue;
                    }

                    memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                           FINSH_CMD_SIZE);
                    shell->line_curpos = shell->line_position = strlen(shell->line);
                    shell_handle_history(shell);
                    continue;
                }
                else if (ch == 0x44) 
                {
                    if (shell->line_curpos)
                    {
                        kprintf("\b");
                        shell->line_curpos --;
                    }

                    continue;
                }
                else if (ch == 0x43)
                {
                    if (shell->line_curpos < shell->line_position)
                    {
                        kprintf("%c", shell->line[shell->line_curpos]);
                        shell->line_curpos ++;
                    }

                    continue;
                }

            }

            if (ch == '\r')
            {
                char next;

                if (shell_byte_read(&next) == 1)
                {
                    if (next == '\0') ch = '\r'; 
                    else ch = next;
                }
                else ch = '\r';
            }

            else if (ch == '\t')
            {
                int i;

                for (i = 0; i < shell->line_curpos; i++)
                    kprintf("\b");

 
                shell_auto_complete(&shell->line[0]);

                shell->line_curpos = shell->line_position = strlen(shell->line);

                continue;
            }

            else if (ch == 0x7f || ch == 0x08)
            {

                if (shell->line_curpos == 0)
                    continue;

                shell->line_position--;
                shell->line_curpos--;

                if (shell->line_position > shell->line_curpos)
                {
                    int i;

                    memcpy(&shell->line[shell->line_curpos],
                               &shell->line[shell->line_curpos + 1],
                               shell->line_position - shell->line_curpos);
                    shell->line[shell->line_position] = 0;

                    kprintf("\b%s  \b", &shell->line[shell->line_curpos]);


                    for (i = shell->line_curpos; i <= shell->line_position; i++)
                        kprintf("\b");
                }
                else
                {
                    kprintf("\b \b");
                    shell->line[shell->line_position] = 0;
                }

                continue;
            }

            if (ch == '\r' || ch == '\n')
            {
                shell_push_history(shell);

                {
                    if (shell->echo_mode)
                        kprintf("\n");
                    msh_exec(shell->line, shell->line_position);
                }

                kprintf(FINSH_PROMPT);
                memset(shell->line, 0, sizeof(shell->line));
                shell->line_curpos = shell->line_position = 0;
                break;
            }


            if (shell->line_position >= FINSH_CMD_SIZE)
                shell->line_position = 0;


            if (shell->line_curpos < shell->line_position)
            {
                int i;

                memcpy(&shell->line[shell->line_curpos + 1],
                           &shell->line[shell->line_curpos],
                           shell->line_position - shell->line_curpos);
                shell->line[shell->line_curpos] = ch;
                if (shell->echo_mode)
                    kprintf("%s", &shell->line[shell->line_curpos]);

                for (i = shell->line_curpos; i < shell->line_position; i++)
                    kprintf("\b");
            }
            else
            {
                shell->line[shell->line_position] = ch;
                if (shell->echo_mode)
                    kprintf("%c", ch);
            }

            ch = 0;
            shell->line_position ++;
            shell->line_curpos++;
            if (shell->line_position >= FINSH_CMD_SIZE)
            {
                shell->line_position = 0;
                shell->line_curpos = 0;
            }
        }
    }      
}

