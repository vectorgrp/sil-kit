#!/bin/sh
set -e
echo "participants; messageSize; messageCount; numberMessageSent; duration; bandwidth(KB)"
for i in $@;
do
  awk '
/^- Number of participants:/{participants=$NF}
/^- Message size \(bytes\):/{msgsize=$NF}
/^- Messages per simulation step \(1ms\):/{msgcount=$NF}
/^- Realtime duration:/{duration=$NF}
/^- Total number of messages:/{numsent=$NF}
/^- Throughput:/{throughput=$3}
  END {
    bandwidth=(msgsize*numsent)/duration
    bandwidth/=1024.0
    printf("%d; %d; %d; %d; %.3f; %.3f;\n",participants, msgcount, msgsize, numsent, duration, throughput )
  }' $i 
done | sed 's;\.;,;g' | sort

