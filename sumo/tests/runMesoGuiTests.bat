set TEXTTEST_HOME=%~dp0
set SUMO_BINARY=%~dp0\..\bin\meso%1.exe
set GUISIM_BINARY=%~dp0\..\bin\meso-gui%1.exe
%TEXTTESTPY% -a sumo.meso.gui
