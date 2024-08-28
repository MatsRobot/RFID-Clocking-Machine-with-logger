set local
@echo off
cls

for /f "tokens=1-5*" %%1 in ('vol') do (
    set volume=%%6 & goto echoit
)
:echoit

set year=%date:~6,4%
set month=%date:~3,2%
set day=%date:~0,2%
set timestamp="%volume%- %year%%month%%day%"

copy ESIGN.CSV %userprofile%\Desktop\eSign-%timestamp%.CSV
