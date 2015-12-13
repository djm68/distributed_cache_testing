#!/bin/bash

create_obj()
{
	./create_obj -o $CREATE_OBJ -b $BYTES
}

destroy_obj()
{
	./destroy_obj -o $CREATE_OBJ
}

restart_cache()
{
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -stop all > /dev/null"
	sleep 5
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -start all > /dev/null"
	sleep 5
}
########################################
# Wall times get tests 								 #
########################################
wall_time_get()
{
	CREATE_OBJ=1000000			# Num objs to create
	GET_OBJ=100000					# Num objs to get
	BYTES=4096							# Object size
	restart_cache           # Restart cache
	create_obj

	echo "*** Walltime GET 100,000 4KB Objects ***"
	time $TG_BIN -o100000 -b4096
	echo "*** Done ***"; echo ; echo

	echo "*** Walltime GET 250,000 4KB Objects ***"
	time $TG_BIN -o250000 -b4096
	echo "*** Done ***"; echo ; echo
}


########################################
# Multi threaded Get test              #
########################################
multi_thread_get()
{
	CREATE_OBJ=10000				# Num objs to create
	GET_OBJ=1000						# Num objs to get
	BYTES=4096							# Object size
	restart_cache           # Restart cache
	create_obj

	echo "*** Multi Threaded Get Test - Fixed Object Size $BYTES Bytes ***" ; echo
	for THRDS in  1 2 4 8 16 32 64  ; do
		echo "$THRDS Threads Timing Run"
		$TG_BIN -o $GET_OBJ -b $BYTES -p $THRDS | grep Ave
		echo; sleep 3
	done
	echo "*** Done ***"; echo ; echo
}

########################################
# Variable Object Size Get             #
########################################
variable_obj()
{
	CREATE_OBJ=10000
	GET_OBJ=1000
	restart_cache
	echo -n "*** Variable Object Size Get Latency Test - "
	if [ $RESTART -eq 0 ]; then  
		echo "Without Cache Restarts ***"; echo
	elif [  $RESTART -eq 1 ]; then
		echo "With Cache Restarts ***"; echo
	fi
	for BYTES in 4096 8192 16384 32768 65536 131072 262144 524288 1048576; do
		echo "$BYTES Bytes Object Get Latency Run"
		create_obj

		$TG_BIN -o $GET_OBJ -b $BYTES | grep Ave
		echo; sleep 3

		if [ $RESTART -eq 0 ]; then  # Restart cache or destory obj before next pass
			destroy_obj
		elif [  $RESTART -eq 1 ]; then
			restart_cache
		fi
	done
	echo "*** Done ***"; echo ; echo
}

########################################
# Version info                         #
########################################
get_info()
{
	echo "*** Object Cache Performance Test Base Line ***"
	date
	ssh $MGR_NODE "cat /opt/rnanetworks/cfm_data/data/global/status.dat | grep Version"
	echo; echo
}

########################################
# MAIN                                 #
########################################
# Set up logging
LOG=oc_perf_`date +%b.%d.%Y`
exec 6>&1 # redir stdout to fd 6
exec > $LOG # set output to logfile
exec 2>&1 # redir stderr to stdout

# Basic test config
MGR_NODE="kr10"
TG_BIN="./time_get"

# Start tests
get_info

# Multi thread, fixed obj
multi_thread_get

# Run variable object size test
RESTART=0
variable_obj
RESTART=1
variable_obj

# Run wall timed tests
wall_time_get
