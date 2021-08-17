#!/bin/sh
set -e
echo "participants; messageSize; messageCount; numberMessageSent; duration; bandwidth(KB)"
for i in $@;
do
	awk '
	/^Number of participants/{participants=$5}
	/^Message size in bytes/{msgsize=$6+0}
	/^Message count per simulation task/{msgcount=$7+0}
	/^Average realtime duration.*/{duration=$4}
	/^Average number of messages/{numsent=$5+0}
	END {
		bandwidth=(msgsize*numsent)/duration
		bandwidth/=1024.0
		printf("%d; %d; %d; %d; %.5f; %.3f\n",participants, msgsize, msgcount, numsent, duration, bandwidth )
	}' $i 
done | sed 's;\.;,;g' | sort

