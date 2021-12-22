import datetime
Import("env")


curr_date = datetime.datetime.now()
env.Append(CPPDEFINES=[
  ("BUILD_DATE", curr_date.strftime("%d.%m.%Y %H:%M:%S"))
])