# HelloMQTT2Way  -  MQTT client with 2 way auth 

 


## How to create certs
<https://github.com/ARMmbed/data-emq-broker>
 
 
# How to create the development env on MacBook
 
## OpenSSL
    brew install openssl
    export PATH=/usr/local/opt/openssl/bin:$PATH
 
## MBED CLI
https://github.com/ARMmbed/mbed-cli
https://os.mbed.com/docs/v5.8/tools/configuring-mbed-cli.html
 
Mbed cli requires python 2.7 or python 3.6  (I use python 2.7)

    pip install mbed-cli
The pip command above might fail (the error message says  which packages are missing).  To fix it I had to run:

    sudo easy_install nose
    sudo easy_install tornado
    pip install mbed-cli
    pip install mbed-cli –upgrade 
 
Install git and mercurial clients:

    brew install mercurial  
Test what mercurial is installed but typing:

    hg
 
## GNU ARM compiler
<https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>
 
Download, unzip and configure path to GNU ARM compiler:

    mbed config -G ARM_PATH  <folder where compiler is located>
On my machine it is  /Users/miclub01/gcc-arm-none-eabi-7-2017-q4-major/bin

    mbed config –list      # show default config
On my machine the output is
[mbed] Global config:
GCC_ARM_PATH=/Users/miclub01/gcc-arm-none-eabi-7-2017-q4-major/bin
 
[mbed] Local config (/Users/miclub01/GIT/HelloMQTT2Way):
TOOLCHAIN=GCC_ARM
TARGET=K64F
 

## Download dependencies

For the new project downloaded from github you might need to run it once:

    mbed deploy
    mbed new .
 

 ## Special file commited in this repo: config.h
 This commited  file is very special:
 ./mbed-os/features/mbedtls/inc/mbedtls/config.h
It is tweaked version of original file, where the following lines are uncommented:

    #define MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3
    #define MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
    #define MBEDTLS_SHA1_C
    #define MBEDTLS_SSL_MAX_CONTENT_LEN             16384
    #define MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_CERTIFICATES

This is not the recommended way to customize the mbed app. Look here for details:
<https://os.mbed.com/docs/v5.8/reference/configuration.html>

The impact of commiting  config.h in this repo is:
it is conflicting with original file and the folowing command generates the error because it cannot download mbed-os into folder with config.h:

    mbed deploy
 [mbed] ERROR: Library reference "mbed-os.lib" points to a folder "/Users/miclub01/CLONE/HelloMQTT2Way/mbed-os", which is not a valid repository.

To fix it please copy this file somewhere outside this project, remove folder mbed-os and run "mbed deploy" again.
After that you can apply the tweak to this file again.

 
 
## Compile, deploy and run mbed project
    mbed compile -t GCC_ARM -m K64F
Deploy (board should be visible in Finder  as /Volumes/DAPLINK/):

    cp ./BUILD/K64F/GCC_ARM/HelloMQTT2Way.bin /Volumes/DAPLINK/

Make sure your MQTT broker is running, you also might want to see the output in seral port using minicom (described below).
Run:  press the button on board
 
## Mosquittto MQTT Broker
 
    brew install mosquitto
Edit  /usr/local/etc/mosquitto/mosquitto.conf

    bind_address 10.72.153.34
    port 8883
    cafile /Users/miclub01/GIT/data-emq-broker/conf/rootCA/MyRootCA.pem 
    certfile /Users/miclub01/GIT/data-emq-broker/conf/serverCerts/10.72.153.34/MQTTBroker.pem
    keyfile /Users/miclub01/GIT/data-emq-broker/conf/serverCerts/10.72.153.34/MQTTBroker.key
 
Start Mosquito broker:

    brew services start mosquitto
 
 
If IP adderess of MQTT broker host changes then
 
a) Update bind_address in /usr/local/etc/mosquitto/mosquitto.conf
b) Generate new  Server cert:

    mkdir 10.72.153.34
    cd 10.72.153.34
    openssl genrsa -out MQTTBroker.key 2048
    openssl req -new -key ./MQTTBroker.key -out MQTTBroker.csr
    openssl req -new -key ./MQTTBroker.key -out MQTTBroker.csr
    openssl x509 -req -in ./MQTTBroker.csr -CA ../../rootCA/MyRootCA.pem -CAkey ../../rootCA/MyRootCA.key -CAcreateserial -out MQTTBroker.pem  -days 3650 -sha256
 
 
You may want to install MQTT client.  I use MQTT.fx    http://mqttfx.jensd.de/
 
 
### minicom 
    brew install minicom
    ls /dev/tty.usb*.       # find the tty name
    minicom -D /dev/tty.usbmodem14412
 
