echo "This script has the purpose of launching mosquitto on the destination server"
echo "Please enter in the following order :"
echo "- IP"
echo "- USER NAME"
echo "- DIRECTORY WHERE YOU WANT TO COPY THE DATA"
echo "- PORT YOU WANT TO LAUNCH THE BROKER"
echo ""
echo "So this script should look like : ./server.sh IP USER DIRECTORY"
echo ""
PORT=test
if [[ -z $3 ]]; then
    echo "Not enough arguments given, default value will be set"
    export IP_PI_PROJECT=192.168.106.57
    export USER_PI_PROJECT=louis
    export DIR_PI_PROJECT=projet
    export PORT=2000
    scp -r  subscriber.c server/ ssh_script.sh $USER_PI_PROJECT@$IP_PI_PROJECT:$DIR_PI_PROJECT 
    ssh $USER_PI_PROJECT@$IP_PI_PROJECT "export PORT=$PORT && cd /home/louis/$DIR_PI_PROJECT && echo $pwd && chmod +x ssh_script.sh && ./ssh_script.sh"
else
    export IP_PI_PROJECT=$0
    export USER_PI_PROJECT=$1
    export DIR_PI_PROJECT=$2
    export PORT=$3
    scp -r  subscriber.c server/ ssh_script.sh $USER_PI_PROJECT@$IP_PI_PROJECT:$DIR_PI_PROJECT
    ssh $USER_PI_PROJECT@$IP_PI_PROJECT "export PORT=$PORT && cd /home/louis/$DIR_PI_PROJECT && echo $pwd  && chmod +x ssh_script.sh && ./ssh_script.sh"
fi