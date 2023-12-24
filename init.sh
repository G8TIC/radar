#
# init.sh - determine init system
#
INIT=`lsof -a -p 1 -d txt | grep txt | cut -f 1 -d " "`

echo "This system uses $INIT"


