#!/bin/bash

echo -e "Warning: this test can result in removing your current ip from your network interface and/or in sending DHCP_RELEASE with your current ip\nBe ready to reconfigure your ip or restart your pc after tests\nAlso you can perform this test without configured ip";
echo "Do you want to do test?";

select confirm in "Y" "N"; do break; done;

if [ $confirm = "Y" ]; then

	PASS="\e[0;32mPASS\e[0m:";
	FAIL="\e[0;31mFAIL\e[0m:";

	#First test
	$1 --arp-check -d -i $2;
	RETURN=$?
	#-2 = 254
	if [ $RETURN == 254 ]; then 
		echo -e "$PASS: arp check";
	else
		echo -e "$FAIL: arp check";
	fi

	if [ -e ./tmp/dhcp_discover.bin ]; then 
		echo -e "$PASS: debug files";
		rm ./tmp/dhcp_discover.bin;
	else
		echo -e "$FAIL: debug files";
	fi

	#second test
	$1 -d --no-debug-files --dry -i $2;
	RETURN=$?
	if [ $RETURN ]; then
		echo -e "$PASS: exit code";
	elif [ $RETURN = -2 ]; then
		echo "Exit code -2, may be got DHCP_NAK";
	else
		echo -e "$FAIL: exit code";
	fi

	if [ ! -e ./tmp/dhcp_discover.bin ]; then
		echo -e "$PASS: --no-debug-files";
	else
		echo -e "$FAIL: --no-debug-files";
	fi

	#Third test
	DHCP_RESULT=$($1 -i $2);
	IP=$(echo $DHCP_RESULT | cut -d: -f1);
	MASK=$(echo $DHCP_RESULT | cut -d: -f2);
	GATEWAY=$(echo $DHCP_RESULT | cut -d: -f3);

	ip addr show dev $2 | grep -F "$IP";
	RETURN=$?
	if [ $RETURN ]; then
		echo -e "$PASS: ip setting";
	else
		echo -e "$FAIL: ip setting";
	fi

	ip route show dev $2 | grep -F "$GATEWAY";
	RETURN=$?
	if [ $RETURN ]; then
		echo -e "$PASS: route setting";
	else
		echo -e "$FAIL: route setting";
	fi

	#4-th test
	$1 --release -i $2;
	ip addr show dev $2 | grep -F "$IP";
	RETURN=$?
	if [ $RETURN == 1 ]; then
		echo -e "$PASS: ip releasing";
	else
		echo -e "$FAIL: ip releasing";
	fi
fi


