::Small batch file that takes all the files in ./raytracer and concatenates them
::@echo off
chdir /D "%~dp0"
del comp.glsl
::Append headers
cd raytracer
for %%f in (*.h) do (type "%%f" >> ../comp.glsl & echo. >> ../comp.glsl)
::Append glsl
for %%f in (*.glsl) do (type "%%f" >> ../comp.glsl & echo. >> ../comp.glsl)