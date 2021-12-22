import datetime
Import("env")


curr_date = datetime.datetime.now()
year = str(curr_date.year)[2:] # ignore first 2 chars of year; 2020 -> 20
# code it in DDMMY
# day and month are zero-padded to length 2; e.g. 021120 -> 02.11.2020
date_str = f"{curr_date.day:02}{curr_date.month:02}{year}"
# append integer value to global defines
env.Append(CPPDEFINES=[
  ("BUILD_DATE", int(date_str))
])