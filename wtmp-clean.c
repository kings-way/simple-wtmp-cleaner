/*
 * wtmp-clean
 *
 * a simple program to erase specific entries in utmp files like /var/log/wtmp, etc.
 *
 * Copyright (C) 2015 King's Way <root@kings-way.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<time.h>
#include<utmp.h>
#include<errno.h>

struct 	ut_node *ut_read_list( char *filename );
char 	*ut_write_list	( char *filename, struct ut_node *head );
void 	ut_print_list	( struct ut_node *head );
void 	ut_print_node	( int id, struct utmp _utmp );
char 	*ut_delete_node	( int id, struct ut_node *head );
void	clear_file		( char *filename);
char 	*get_time		( time_t time );

struct ut_node
{
		int id;
		struct utmp _utmp;
		struct ut_node *next;
};

int main( int argc, char **argv)
{
		if(argc<2)
		{
				printf("Usage:\twtmp-clean [utmp_file] :        to delete specific entry in interactive mode\n");
				printf("      \twtmp-clean clear [utmp_file] :  to clear the data in the file\n");
				printf("\nExample:wtmp-clean /var/log/wtmp\n");
				printf("\twtmp-clean /run/utmp\n");
				printf("\twtmp-clean clear /var/log/btmp\n");
				exit(-1);
		}
		

		else if(argc==3&&strcmp(argv[1],"clear")==0)
		{
				clear_file(argv[2]);
				return 0;
		}
		
		char *filename=argv[1];
		struct ut_node *head=ut_read_list(filename);
		
		int input=-2;
		char *result=NULL;
		do
		{
				system("clear");
				printf("\t\t\t##########################################\n");
				printf("\t\t\t########## a simple wtmp cleaner #########\n");
				printf("\t\t\t##########################################\n\n");
				switch(input)
				{
						case -1:result=ut_write_list(filename,head);break;
						default:
								if(input>=0)
										result=ut_delete_node(input,head);
				}

				ut_print_list(head);
				printf("%s",result==NULL?"":result);
				printf("\nInput the number of the entry to delete: \n");
				printf(" ('-1' to save ;  'q' to exit )\n");

		}while(scanf("%d",&input)>0);

		return 0;
}

struct ut_node *ut_read_list(char *filename)
{	
		int fd;
		if((fd=open(filename,O_RDONLY))<0)
		{
				perror("File open error:");
				exit(-1);
		}
		struct ut_node *head=malloc(sizeof(struct ut_node));
        struct ut_node *cur=head;
        head->id=0;
        head->next=NULL;

        int i=0;
        while(read(fd,&cur->_utmp,sizeof(struct utmp))>0)
        {
                cur->id=i++;
                cur->next=malloc(sizeof(struct ut_node));
                cur=cur->next;
                cur->next=NULL;
        }
		close(fd);
		return head;
}

char *ut_write_list(char *filename, struct ut_node *head)
{
		int fd=open(filename,O_RDWR|O_TRUNC);
		struct ut_node *cur=head;
		while(cur->next!=NULL)
		{
				if(write(fd,&cur->_utmp,sizeof(struct utmp))<=0)
				{
						perror("Write Error:");
						printf("\n");
						exit(-1) ;
				}
				cur=cur->next;
		}
		close(fd);
		return "\n\t\t\tWrite Successfully!\n\n";
}

void ut_print_list(struct ut_node *head)
{
		struct ut_node *cur=head;
		while(cur->next!=NULL)
		{
				ut_print_node(cur->id,cur->_utmp);
				cur=cur->next;
		}
}

void ut_print_node(int id, struct utmp _utmp)
{
    	const char *str_addr, *str_time;
    	char tmp[INET6_ADDRSTRLEN];

    	if (_utmp.ut_addr_v6[1] || _utmp.ut_addr_v6[2] || _utmp.ut_addr_v6[3])
				str_addr= inet_ntop(AF_INET6, &_utmp.ut_addr_v6, tmp, sizeof(tmp));
    	else
        		str_addr= inet_ntop(AF_INET, &_utmp.ut_addr_v6, tmp, sizeof(tmp));

    	str_time= get_time(_utmp.ut_tv.tv_sec);

		char *type=NULL;
		switch(_utmp.ut_type)
		{
				case 1: type="run_lvl";	break;
				case 2:	type="reboot";	break;
				case 6:	type="tty_init";break;
				case 7:	type="LogIN";	break;
				case 8:	type="LogOUT";	break;
		}
    	printf("# %-4d  %8s   %05d  %-*.*s %-*.*s %-*.*s %-15s %-28.28s\n",
          		id,type, _utmp.ut_pid, 8, UT_NAMESIZE, _utmp.ut_user,12,
		  		UT_LINESIZE,_utmp.ut_line, 20, UT_HOSTSIZE, _utmp.ut_host,str_addr, str_time);
}

char *get_time(time_t time)
{
		static char *str=NULL;
		str=malloc(sizeof(char)*29);
    	struct tm *_tm;

    	if (time != 0 && (_tm = localtime(&time)))
        		strftime(str, 29, "%a %b %d %T %Y %Z", _tm);
    	else
        		str[0] = '\0';

    	return str;
}

char *ut_delete_node(int id, struct ut_node *head)
{
		struct ut_node *pre=head;
		struct ut_node *cur=head;
		while(cur->next!=NULL)
		{
			 	if(cur->id==id)
                {
					if(head==cur)
                    	head=cur->next;

                    pre->next=cur->next;
                    free(cur);
					cur=pre->next;

					return "\n\t\t\t Yes! Data Deleted. Don't forget to svae...\n\n";
					
				}
				pre=cur;
				cur=cur->next;
		}
		
		return "\n\t\t\tSorry! Entry Not Exist\n\n";
}
void clear_file(char *filename)
{
		char *cmd=malloc(sizeof(char)*200);
		sprintf(cmd,"getfacl %s > /tmp/aCl && rm -rf %s&&touch %s&&cd /&&setfacl --restore=/tmp/aCl",filename,filename,filename);
		system(cmd);
		system("rm -rf /tmp/aCl");
		// if I run 'echo > /var/log/btmp', then the new data in the future will be a mess...
}

