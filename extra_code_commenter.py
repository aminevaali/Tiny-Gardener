# Run this program before compiling main.cpp
# This program comments/uncomments extra codes that are required only for debugging and not required in deployment.
# e.g Serial.print commands

import os
import re

# User selects to comment or uncomment extra codes
print("Select one option:")
print("1. comment : c")
print("2. uncomment : u")
shouldComment = input()

source = open("src/main.cpp", "r")
destination = open("src/tmp.cpp", "w")
lines = source.readlines()

if shouldComment == "c":
    for line in lines:
        if re.match(r"\s*Serial.print", line): # commenting every command that starts with Serial.print
            line = line.replace("Serial.print", "// Serial.print")
        destination.write(line)
elif shouldComment == "u":
    for line in lines:
        if re.match(r"\s*// Serial.print", line):
            line = line.replace("// Serial.print", "Serial.print") # uncommenting every commented line that starts with // Serial.print
        destination.write(line)
else:
    print("Invalid input!")


source.close()
destination.close()

os.remove("src/main.cpp")
os.rename("src/tmp.cpp", "src/main.cpp")