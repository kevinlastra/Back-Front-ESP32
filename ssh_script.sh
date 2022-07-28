cd /etc/mosquitto/
echo "Installing mosquitto"
sudo apt install mosquitto
echo ""
echo "replacing port in mosquitto.conf"
if grep listener mosquitto.conf ; then
    sudo sed -i "s/.*listener.*/listener $PORT/" mosquitto.conf
else
    sudo bash -c "echo 'listener $PORT' >> mosquitto.conf"
fi
if grep allow_anonymous mosquitto.conf ; then
    sudo sed -i "s/.*allow_anonymous.*/allow_anonymous true/" mosquitto.conf
else
    sudo bash -c "echo 'allow_anonymous true' >> mosquitto.conf"
fi