#Simple Wtmp Cleaner

##Notice:
+ Yeah!  This only contains a C file!
+ Just compile it, and then try it out!


##Usage:
	wtmp-clean [utmp_file] :       	to delete specific entry in interactive mode
	wtmp-clean clear [utmp_file] :  to clear the data in the file

##Example:
	wtmp-clean /var/log/wtmp
	wtmp-clean /run/utmp
	wtmp-clean clear /var/log/btmp
