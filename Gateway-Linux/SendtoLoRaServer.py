import sys
import socket
import subprocess
import struct
import random
import base64

from struct import *

argvs = sys.argv
argc = len(argvs)
print argvs
if (argc < 3):
    print 'Usage: # python %s filename' % argvs[0]
    quit()

server = "58.157.42.117"
port = 19999

print 'server    : %s ' % server
print 'port  : %s ' % port
print 'argvs1: %s ' % argvs[1]

# change your gateway ID(from 0xA8 to 0xD4)
head = chr(1) \
+ chr(random.randint(0,255)) \
+ chr(random.randint(0,255)) \
+ chr(0) \
+ chr(0xA8) \
+ chr(0x40) \
+ chr(0x41) \
+ chr(0xFF) \
+ chr(0xFF) \
+ chr(0x18) \
+ chr(0x1B) \
+ chr(0x00)

if (argvs[1] == "stat"):
    print'Send Status'
    stat = "{\"stat\":{" \
      + "\"time\":" + subprocess.check_output(["date", "+\"%Y-%m-%d %H:%M:%S GMT"])[0:24] + "\"," \
      + "\"lati\":139.6315," \
      + "\"long\":35.479," \
      + "\"alti\":0," \
      + "\"rxnb\":0," \
      + "\"rxok\":0," \
      + "\"rxfw\":0," \
      + "\"ackr\":0," \
      + "\"dwnb\":0," \
      + "\"txnb\":0," \
      + "\"pfrm\":\"Dragino LG01-JP\"," \
      + "\"mail\":\"lijihang@neusoft.co.jp\"," \
      + "\"desc\":\"\"" \
      + "}}"
    head+=stat
else:
    print'Send Data'
    rxpk = "{\"rxpk\":[{" \
      + "\"tmst\":" + subprocess.check_output(["date", "+%s"])[0:10] + ",".replace('\n','') \
      + "\"chan\":0," \
      + "\"rfch\":0," \
      + "\"freq\":" + argvs[3] + "," \
      + "\"stat\":1," \
      + "\"modu\":\"LORA\"" + "," \
      + "\"datr\":\"SF7" \
      + "BW125\""  + ","\
      + "\"codr\":\"4/5\"," \
      + "\"lsnr\":9," \
      + "\"rssi\":" + argvs[1] + "," \
      + "\"size\":" + argvs[2] + "," \
      + "\"data\":\""
    
    for line in open('/mnt/mtdblock3/data/bin', 'r'):
        itemList = line[:-1].split(';')
    
    print itemList
    print type(itemList[0])

    shidu = str(float(itemList[1]) + float(itemList[2])/100)
    print shidu
    jcwd = str(float(itemList[3]) + float(itemList[4])/100)
    print jcwd

    data1 = "{\"code\":\"WG73\", \"jcwd\":\"" + jcwd + "\", \"fdjzs\":\"" + itemList[0] + "\", \"shidu\":\"" + shidu + "\"}"
    print(data1)
    rxpk+=base64.b64encode(data1)
    rxpk+="\"}]}"
    head+=rxpk
    print(head)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(bytes(head), (server, port))
sock.close()
    
'''
    f = open('/mnt/mtdblock3/data/bin','rb')
    data1 = f.read()
    f.close()

    

f = open('/mnt/mtdblock3/data/send','w')
f.write(head)
f.close()
print(head)

'''