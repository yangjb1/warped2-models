#!/bin/bash
# Allows batch runs of simulations. Saves results to log files

# run if user hits control-c
function control_c() {
    echo -en "*** Ouch! Exiting ***\n"
    exit $?
}

function run {
    testCycles=$1
    timeoutPeriod=$2
    model=$3
    modelCmd=$4
    maxSimTime=$5
    workerThreads=$6
    scheduleQCount=$7
    groupSize=$8

    for ((iteration=1; iteration <= $testCycles; ++iteration));
    do
        cd ../models/$model/
        outMsg="\n($iteration/$testCycles) $modelCmd : $workerThreads threads, \
                $scheduleQCount schedulers, group size: $groupSize, max sim time: $maxSimTime"
        echo -e $outMsg
        tmpFile=`tempfile`
        runCommand="$modelCmd --max-sim-time $maxSimTime --simulation-type time-warp \
            --time-warp-worker-threads $workerThreads --time-warp-scheduler-count $scheduleQCount \
            --time-warp-group-size $groupSize \
            --time-warp-gvt-calculation-period 2600 --time-warp-statistics-file $tmpFile"
        timeout $timeoutPeriod bash -c "$runCommand" | grep -e "Simulation completed in " -e "Type of Schedule queue: "

        # Parse stats
        statsRaw=`cat $tmpFile | grep "Total,"`
        statsRefined=`echo $statsRaw | sed -e 's/Total,//g' -e 's/\t//g' -e 's/ //g'`

        rm $tmpFile
        cd ../../scripts/

        # Write to log file
        echo "$model,$modelCmd,$workerThreads,$scheduleQCount,$groupSize,$statsRefined" >> $logFile

        sleep 10
    done
}

hostname=`hostname`
date=`date +"%m-%d-%y_%T"`
logFile="logs/$hostname---$date.csv"

# Write csv header
csvHeader="Model,ModelCommand,WorkerThreadCount,ScheduleQCount,GroupSize,Runtime,NumObjects,LocalPositiveEventsSent,RemotePositiveEventsSent,LocalNegativeEventsSent,RemoteNegativeEventsSent,PrimaryRollbacks,SecondaryRollbacks,CoastForwardedEvents,CancelledEvents,EventsProcessed,EventsCommitted,AvgMaxMemory"
echo $csvHeader > $logFile

trap control_c SIGINT

. $1

