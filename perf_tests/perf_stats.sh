#!/bin/bash

create_obj()
{
	${WRKDIR}/create_obj -o $CREATE_OBJ -b $BYTES
}

destroy_obj()
{
	${WRKDIR}/destroy_obj -o $CREATE_OBJ
}

stop_cache()
{
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -stop all"
	sleep 2
}

restart_cache()
{
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -stop all > /dev/null"
	sleep 5
	ssh $MGR_NODE "cd /opt/rnanetworks/bin && /opt/rnanetworks/bin/control.pl -start all > /dev/null"
	sleep 5
}

uninst()
{
	ssh $MGR_NODE "cd  /opt/rnanetworks/bin && ./uninstall.pl all" 
	sleep 1
	#ssh $MGR_NODE "rm -rf  /opt/rnanetworks" 
	sleep 1
	for node in ${NODES[@]}; do
		ssh $node "rpm -e librnaclient-2.5-0.sabre_poc_2 librnaclient-devel-2.5-0.sabre_poc_2"
		ssh $node "rm -f librnaclient-2.5-0.sabre_poc_2.x86_64.rpm librnaclient-devel-2.5-0.sabre_poc_2.x86_64.rpm"
	#	ssh $node "rm -rf  /opt/rnanetworks" 
	done
	sleep 1
}

install_new()
{
	cd $TAG && cd RNA-2.5.0.sabre_poc_2*
	cp -f ${WRKDIR}/rna.conf .
	./install.pl all
  scp rna.conf $MGR_NODE:/opt/rnanetworks/bin
  scp VERSION $MGR_NODE:/opt/rnanetworks/bin
 
	for node in ${NODES[@]}; do
		scp librnaclient-2.5-0.sabre_poc_2.x86_64.rpm	${node}:
		scp librnaclient-devel-2.5-0.sabre_poc_2.x86_64.rpm ${node}:
		ssh $node "rpm -ih librnaclient-2.5-0.sabre_poc_2.x86_64.rpm librnaclient-devel-2.5-0.sabre_poc_2.x86_64.rpm"
	done
	cd ${WRKDIR}
	sleep 2
}

########################################
# Multi threaded Get test              #
########################################
multi_thread_get()
{
  echo "# pass ${pass}"
	restart_cache			# Restart cache
	create_obj
	$TG_BIN -o $GET_OBJ -b $BYTES -p $THRD | grep Average | cut -d":" -f2
	sleep 3
}

# Prep test environment before starting actual tests
test_prep()
{
	#version=`ssh ${MGR_NODE} "cat /opt/rnanetworks/cfm_data/data/global/status.dat | grep Version  | cut -d\" \" -f2"`
	#mkdir ${version}_`date +%b.%d.%Y`  && cd ${version}_`date +%b.%d.%Y`

	mkdir ${TAG}_`date +%b.%d.%Y` && cd ${TAG}_`date +%b.%d.%Y`
	echo "RNA Cache Version ${TAG}" > testinfo.txt
	date >> testinfo.txt
	echo "Statistical Profile - Multi Threaded Get Latency" >> testinfo.txt
	echo "${CREATE_OBJ} object - ${BYTES}Kb fixed size - ${GET_OBJ} GET operations" >> testinfo.txt
}

# Post process - generate stats from raw timings
post_process()
{
	for THRD in ${THRDS[@]}; do
		echo "# ${THRD} Threads" >> stats_output
		echo -n "${THRD} " >> stats_output
		cat	${THRD}_thrds_ocperf | grep -v pass  | dbstats get_latency | grep -v "#" >> stats_output
	done

	# Put all the pieces together into a single csv file
	cat testinfo.txt > stats_output.csv
	echo >>  stats_output.csv
	echo "Threads,Mean,Std Dev,Pct_rsd,Conf_range,Conf_low,Conf_high,Conf_pct,Sum,Sum_squared,Min,Max,N" >> stats_output.csv
	cat stats_output | grep -v "#" | tr " " , >> stats_output.csv
}


########################################
# MAIN                                 #
########################################

########################
# Start Config Section #
########################

MGR_NODE="kr10"		# cfm node
NODES=( kr11 kr12 kr13 kr14 kr15 kr16 )	# list of client nodes

#THRDS=( 4 8 16 32 64 )	# test passes with n threads each
THRDS=( 64 )	# test passes with n threads each
CREATE_OBJ=100000	# Num objs to create
GET_OBJ=10000			# Num objs to get
BYTES=4096				# Object size

########################
# End Config Section   #
########################


WRKDIR=`pwd`
TG_BIN="${WRKDIR}/time_get"
TAG=$1

# Prepare test env
test_prep

# Install new tag
do_new_build()
{
	stop_cache
  uninst	
  install_new
}

# Main loop: n threads, x passes
pass=1
for THRD in ${THRDS[@]}; do
	LOG=${THRD}_thrds_ocperf	# Setup log
	exec 6>&1		# redir stdout to fd 6
	exec > $LOG	# set output to logfile
	exec 2>&1		# redir stderr to stdout
	echo "#fsdb get_latency time_scale"

	pass=1
	while [ ${pass} -le 10 ]; do
		multi_thread_get
		let pass=${pass}+1
	done
done

# process raw timing into csv files
post_process
