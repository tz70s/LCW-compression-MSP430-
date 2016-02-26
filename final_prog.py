import serial
import time
import os

ser = serial.Serial('COM3', 9600, timeout=2)


src_file = 'data.txt'
dst_file = 'compress.txt'
decompress_file = 'decompress.txt'

while 1:
    print("------------------------------------------------------------")
    print()
    print("Lossless Compression / Decompression")
    print()
    print("------------------------------------------------------------")
    print("[Lossless] Compress or Decompress? Please enter 'c' or 'd'")
    print("[Lossless] Enter 'e' to leave.")
    ans = input(">> ")
    if ans == 'e' or ans == 'E':
        exit(1)
    if ans == 'c' or ans == 'C':
        try:
            f = open(src_file,'r',encoding='utf-8')
        except:
            print ("File is not existed!")
            exit(1)
    elif ans == 'd' or 'D':
        try:
            f = open(dst_file,'r')
        except:
            print ("File is not existed!")
            exit(1)
        
    in_string = f.readline()
    f.close()
    if ans == 'c' or ans == 'C':
        in_string += '.'
    print("Please wait for ",len(in_string)," sec.")
    out = ''
    print(">> ",end="")
    for char in in_string:
        ser.write((char).encode())
        print('=', end='')
        time.sleep(0.8)
        while ser.inWaiting() > 0:
            temp = ser.read(1).decode('ascii','ignore')
            out += temp
    if out != '':
        if ans == 'c' or ans == 'C':
            with open(dst_file,'w',encoding = 'ascii') as w:
                w.write(out)
            print()
            src_info = os.stat(src_file)
            dst_info = os.stat(dst_file)
            print(">> File size before : " , src_info.st_size , " bytes. ")
            print(">> File size after : " , dst_info.st_size , " bytes. ")
            print(">> Compress ratio : " , round((dst_info.st_size/src_info.st_size),3))
        elif ans == 'd' or ans == 'D':
            wr = ''
            for ch in out:
                if ch <= 'z' and ch >= 'a':
                    wr += ch
            with open(decompress_file,'w',encoding = 'ascii') as w:
                w.write(wr)
            print()
            src_info = os.stat(src_file)
            dst_info = os.stat(dst_file)
            print(">> File size decompress before : " , dst_info.st_size, " bytes. ")
            print(">> File size decompress after : " , src_info.st_size, " bytes. ")
            #print(">> Compress ratio : " , round((dst_info.st_size/src_info.st_size),3))
    print()
    

ser.close()
