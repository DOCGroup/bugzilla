#!/bin/sh

CONF=client.conf

# To see how test works without FT_Client_ORB, uncomment the following line
#CONF=/dev/null

# Start two servers
./server -ORBEndPoint iiop://:9220 &
./server -ORBEndPoint iiop://:9221 &

# Wait for servers to settle
sleep 2

# Execute the test. If all is fine, client will exit(0), and both servers
# would be terminated
echo "corbaloc:iiop:localhost:9220,iiop:localhost:9221/CorbaSimple" | \
    ./client -ORBSvcConf $CONF
RV=$?

if [ $RV -ne 0 ]; then
    # Run the command again, in order to terminate the remaining server
    # ignore the output
    echo "corbaloc:iiop:localhost:9220,iiop:localhost:9221/CorbaSimple" | \
	./client -ORBSvcConf $CONF >/dev/null
fi

# Return the error code to the caller
[ $RV -eq 0 ]
