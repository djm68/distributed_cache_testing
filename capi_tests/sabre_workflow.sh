#!/bin/bash

restart_cache()
{
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -stop all > /dev/null"
	sleep 5
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -start all > /dev/null"
	sleep 5
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
# Sabre Put/Get/Destroy work flow      #
########################################
work_flow()
{
	# Start spinning cycle for client gets
	ssh ${NODES[1]} "cd $BIN_PATH  && ./spin_get.sh &" &
	ssh ${NODES[2]} "cd $BIN_PATH  && ./spin_get.sh &" &
	ssh ${NODES[3]} "cd $BIN_PATH  && ./spin_get.sh &" &
	ssh ${NODES[4]} "cd $BIN_PATH  && ./spin_get.sh &" &

  i=1 
  while [ 1 ]; do
		# Put objects into cache
		echo; echo "*** START LOOP $i ***"; echo
		ssh ${NODES[0]} "cd $BIN_PATH && $VERB_BIN -p1 -o100000 -b16384"
		sleep 5
		ssh ${NODES[5]} "cd $BIN_PATH && $VERB_BIN -d1 -o100000 -b16384"
		let i=$i+1
	done
}

########################################
# Version info                         #
########################################
get_info()
{
	echo "*** Sabre Workflow Test***"
	date
	ssh $MGR_NODE "cat /opt/rnanetworks/cfm_data/data/global/status.dat | grep Version"
	echo; echo
}

########################################
# MAIN                                 #
########################################
# Set up logging
LOG=workflow_test_`date +%b.%d.%Y`
exec 6>&1 # redir stdout to fd 6
exec > $LOG # set output to logfile
#exec 2>&1 # redir stderr to stdout

# Basic test config
MGR_NODE="kr10"
declare -a NODES=( kr11 kr12 kr13 kr14 k15 kr16 )
BIN_PATH="/root/oc_test"
VERB_BIN="./verbs_para"

# Start tests
get_info

work_flow
