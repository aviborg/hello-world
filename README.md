# hello-world
A simple proof of concept on how to interact with openhab2 from C++ code via the REST api. The program will fetch all items with tag dummyItem and set their state to the some random number. Copy the file in the items folder to the items folder of your openhab2 installation (default: /etc/openhab2/items)

The code uses two additional libraries: jsmn and curl, see the wiki on how to insatll these.

It is only tested on a raspberry pi 3 with openhabianpi installed. If openhab is installed on some other host you need to change the url from localhost to whatever hostname you have.

When experimenting it is a good idea to install the REST Documentation (found under addins MISC in the paper ui), then you can test things out int the REST API panel.
