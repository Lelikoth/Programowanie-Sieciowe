#!/usr/bin/env python3

import os, os.path, socket, sys
import random as rand

prog_name = os.path.basename(sys.argv[0])

if len(sys.argv) != 5:
    print(f"Usage: {prog_name} HOST PORT TEST_NUM MAX_INT", file=sys.stderr)
    sys.exit(1)

def random_equation_generator(max = sys.maxsize//7):
    number_to_generate = rand.randint(1,7)
    equation = ""
    result = 0
    for i in range(number_to_generate):
        random = rand.randint(0, max)
        if equation == "":
            equation = '0' * rand.randint(0,10) + str(random)
            result = random
        else:
            sign = rand.randint(0, 1)
            if (sign):
                equation += '+' + str(random)
                result += random
            else:
                equation += '-' + str(random)
                result -= random
    return test_data(equation, str(result))

def random_equation_overflow_generator(max = sys.maxsize):
    number_to_generate = rand.randint(1,7)
    equation = ""
    result = 0
    overflow = False
    for i in range(number_to_generate):
        random = rand.randint(0, max)
        if equation == "":
            equation = '0' * rand.randint(0,10) + str(random)
            result = random
        else:
            sign = rand.randint(0, 1)
            if (sign):
                equation += '+' + str(random)
                result += random
                if result > max or result < -max - 1:
                    overflow = True
            else:
                equation += '-' + str(random)
                result -= random
                if result > max or result < -max - 1:
                    overflow = True
    if overflow:
        return test_data(equation, ["ERROR", "ERROR\r\n"])
    return test_data(equation, str(result))

server_addr = ( sys.argv[1], int(sys.argv[2]) )
def test_data(text, good, printer = 0):
    client_request = str.encode(text)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.connect(server_addr)
    if sock.send(client_request) != len(client_request):
        print(f"{prog_name}: partial send", file=sys.stderr)
        sys.exit(1)
    sock.settimeout(5.0)
    server_reply = sock.recv(65536)
    sock.close()
    if server_reply.decode() not in good:
        if isinstance(good, list):
            good = [str.encode(s) for s in good]
        else:
            good = str.encode(good)
        print (f"Test '{str.encode(text)}' failed. Expected: '{good}', received: '{server_reply}'\n")
        return False
    if printer:
        print (f"Test '{str.encode(text)}' succeed.\n")
    return True

#Tests
print("-------------------TESTS WITH RANDOM EQUATIONS-----------------------\n")
test_num = int(sys.argv[3])
passed = 0
for i in range (test_num):
    if random_equation_generator(int(sys.argv[4])//7):
        passed += 1
print(f"Passed {passed} out of {test_num} random tests without overflow")

passed = 0
for i in range (test_num):
    if random_equation_overflow_generator(int(sys.argv[4])):
        passed += 1
print(f"Passed {passed} out of {test_num} random tests with possible overflow\n")

print("-------------------TESTS WITH BORDER SIZE AND ZERO-----------------------\n")

passed = 0
if test_data(sys.argv[4], [f"{sys.argv[4]}", f"{sys.argv[4]}\r\n"], 1):
    passed += 1
if test_data(f"0-{sys.argv[4]}-1", [f"{-int(sys.argv[4])-1}", f"{-int(sys.argv[4])-1}\r\n"], 1):
    passed += 1
if test_data(f"0", [f"0", f"0\r\n"], 1):
    passed += 1
if test_data(f"000000000", [f"0", f"0\r\n"], 1):
    passed += 1
print(f"Passed {passed} out of 4 border and zero tests\n")

print("-------------------TESTS WITH ERROR-----------------------\n")
tests = ["", "+1238-3921", "-1238-3921", "1238--3921", "1238+-3921", "1238-+3921", "1238++3921", "1238-3921+",
         "1238+3921-", "10000+32132\r\n+3123", "StrIng+Number", "\r\n", f"{int(sys.argv[4]) + 1}", "-100",
         " 2137", "     2137", "2137 ", "2137       ", " 2137 ", "      2137       ", "10000*32132",
         "10000/32132", "10000+32132\0", "\0"]

passed = 0

for item in tests:
    if test_data(item, ["ERROR", "ERROR\r\n"], 1):
        passed += 1
print(f"Passed {passed} out of {len(tests)} tests with expected ERROR\n")

print("-------------------TESTY Z PRACOWNI-----------------------\n")
def create_sum():
    string = ""
    for i in range (251):
        string += str(i) + '+'
    return string[0:-1]

testy = ["2+2", "20-8+4", "5-7", "72-280+13-0-25+3", "45+1044-512+0+28-0077", create_sum(), "2 + 2", " 2+2", "2+2 ", "1+2.5+7",
         "0x6-4", ''.join([chr(50), chr(226), chr(136), chr(146), chr(50)]), "5+12\0", "77+12345678901234567890+44", 
         ''.join([chr(90), chr(97), chr(197), chr(188), chr(195), chr(179), chr(197), chr(130), chr(196), chr(135), chr(32), chr(103), chr(196), chr(153),
                  chr(197), chr(155), chr(108), chr(196), chr(133), chr(32), chr(106), chr(97), chr(197), chr(186), chr(197), chr(132), chr(46)]), 
         "0+32767", "0+32767+1", "0-32767", "0-32767-1", "0-32767-2", "0+2147483647", "0+2147483647+1", "0-2147483647", "0-2147483647-1", "0-2147483647-2",
         "0+9223372036894775807", "0+9223372036894775807+1", "0-9223372036894775807", "0-9223372036894775807-1", "0-9223372036894775807-2",
         "17+23\n", "17+23\r\n", "17+23\n\r", "17+23\r", "Ala ma kota.\n"]
odpowiedzi = ["4", "16", "-2", "-217", "528", "31375", "ERROR", "ERROR", "ERROR", "ERROR", "ERROR", "ERROR", "ERROR", "ERROR", "ERROR", "32767", 
              ["32768", "ERROR"], "-32767", "-32768", ["-32769", "ERROR"], ["2147483647", "ERROR"], ["2147483648", "ERROR"], ["-2147483647", "ERROR"],
              ["-2147483648", "ERROR"], ["-2147483649", "ERROR"], ["9223372036894775807", "ERROR"], "ERROR", ["-9223372036894775807", "ERROR"],
              ["-9223372036894775808", "ERROR"], "ERROR", ["40", "40\r\n", "ERROR"], ["40", "40\r\n", "ERROR"], "ERROR", "ERROR", "ERROR"]
passed = 0
for i in range(len(testy)):
    if test_data(testy[i], odpowiedzi[i], 1):
        passed += 1
print(f"Passed {passed} out of {len(testy)} Palacz tests\n")
# W powyższym kodzie nie ma obsługi tych błędów, dla których jądro systemu
# zwraca -1. Python te błędy zamienia na wyjątki, a skoro powyższy kod
# wyjątków nie przechwytuje, to działanie programu zostanie przerwane
# w razie wystąpienia błędu z tej kategorii.
