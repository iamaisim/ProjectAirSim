@Rem Copyright Epic Games, Inc. All Rights Reserved.

@echo off

@Rem Set script location as working directory for commands.
pushd "%~dp0"

@Rem Turned on delayed expansion so we have variables evaluated in nested scope
setlocal EnableDelayedExpansion

@Rem Unset all our variables as these persist between cmd sessions if cmd not closed.
SET "PSInfraOrg="
SET "PSInfraRepo="
SET "PSInfraTagOrBranch="
SET "ReleaseVersion="
SET "ReleaseUrl="
SET "IsTag="
SET "RefType="
SET "Url="
SET "DownloadVersion="
SET "FlagPassed="

:arg_loop_start
SET ARG=%1
if DEFINED ARG (
    if "%ARG%"=="/h" (
        goto print_help
    )
    if "%ARG%"=="/v" (
        SET UEVersion=%2
        SET FlagPassed=1
        SHIFT
    )
    if "%ARG%"=="/b" (
        SET PSInfraTagOrBranch=%2
        SET IsTag=0
        SET FlagPassed=1
        SHIFT
    )
    if "%ARG%"=="/t" (
        SET PSInfraTagOrBranch=%2
        SET IsTag=1
        SET FlagPassed=1
        SHIFT
    )
    if "%ARG%"=="/r" (
        SET "ReleaseVersion=%2"
        SET "ReleaseUrl=https://github.com/EpicGames/PixelStreamingInfrastructure/releases/download/!ReleaseVersion!/!ReleaseVersion!.zip"
        SET IsTag=0
        SET FlagPassed=1
        SHIFT
    )
    SHIFT
    goto arg_loop_start
)

@Rem Name and version of ps-infra that we are downloading
SET PSInfraOrg=EpicGames
SET PSInfraRepo=PixelStreamingInfrastructure

@Rem If a UE version is supplied set the right branch or tag to fetch for that version of UE
if DEFINED UEVersion (
  if "%UEVersion%"=="4.26" (
    SET PSInfraTagOrBranch=UE4.26
    SET IsTag=0
  )
  if "%UEVersion%"=="4.27" (
    SET PSInfraTagOrBranch=UE4.27
    SET IsTag=0
  )
  if "%UEVersion%"=="5.0" (
    SET PSInfraTagOrBranch=UE5.0
    SET IsTag=0
  )
  if "%UEVersion%"=="5.1" (
    SET PSInfraTagOrBranch=UE5.1
    SET IsTag=0
  )
  if "%UEVersion%"=="5.2" (
    SET PSInfraTagOrBranch=UE5.2
    SET IsTag=0
  )
)

@Rem If no arguments select a specific version, fetch the appropriate default
if NOT DEFINED PSInfraTagOrBranch (
    SET PSInfraTagOrBranch=UE5.2
    SET IsTag=0
)
echo Tag or branch: !PSInfraTagOrBranch!

@Rem Whether the named reference is a tag or a branch affects the Url we fetch it on
if %IsTag%==1 (
  SET RefType=tags
) else (
  SET RefType=heads
)

@Rem We have a branch, no user-specified release, then check repo for the presence of a RELEASE_VERSION file in the current branch
if %IsTag%==0 (
  if NOT DEFINED ReleaseUrl (
    @Rem We don't want to auto-set the release version if the user passed an explicit flag.
    if NOT DEFINED FlagPassed (
      FOR /F "tokens=* USEBACKQ" %%F IN (`curl -s -f -L https://raw.githubusercontent.com/EpicGames/PixelStreamingInfrastructure/%PSInfraTagOrBranch%/RELEASE_VERSION`) DO (
        SET "ReleaseVersion=!PSInfraTagOrBranch!-%%F"
        SET "ReleaseUrl=https://github.com/EpicGames/PixelStreamingInfrastructure/releases/download/!ReleaseVersion!/!ReleaseVersion!.zip"
      )
    )
  )
)

@Rem Set our DownloadVersion here as we use this to check the contents of our DOWNLOAD_VERSION file shortly.
SET "DownloadVersion=%PSInfraTagOrBranch%"
if DEFINED ReleaseVersion (
  SET "DownloadVersion=!ReleaseVersion!"
  echo Release: !ReleaseVersion!
)

@Rem Check for the existence of a DOWNLOAD_VERSION file and if found, check its contents against our %DownloadVersion%
if exist DOWNLOAD_VERSION (

  @Rem Read DOWNLOAD_VERSION file into variable
  FOR /F "delims=" %%F IN ( DOWNLOAD_VERSION ) DO (
    SET "PreviousDownloadVersion=%%F"
    @Rem Remove whitespace
    SET "PreviousDownloadVersion=!PreviousDownloadVersion: =!"
  )

  if !DownloadVersion! == !PreviousDownloadVersion! (
    echo Downloaded version ^(!DownloadVersion!^) of PS infra matches release version ^(!PreviousDownloadVersion!^)...skipping install.
    goto :EOF
  ) else (
    echo There is a newer released version ^(!DownloadVersion!^) - had ^(!PreviousDownloadVersion!^), downloading...
  )
) else (
  echo DOWNLOAD_VERSION file not found...beginning ps-infra download.
)

@Rem By default set the download url to the .zip of the branch
SET Url=https://github.com/%PSInfraOrg%/%PSInfraRepo%/archive/refs/%RefType%/%PSInfraTagOrBranch%.zip

@Rem If we have a ReleaseUrl then set it to our download url
if DEFINED ReleaseUrl (
  SET Url=!ReleaseUrl!
)

@Rem Download ps-infra and follow redirects.
echo Attempting downloading Pixel Streaming infrastructure from: !Url!
curl -L !Url! > ps-infra.zip

@Rem Unarchive the .zip
tar -xmf ps-infra.zip || echo bad archive, contents: && type ps-infra.zip && exit 0

@Rem Remove old infra
if exist Frontend\ ( rmdir /s /q Frontend )
if exist Matchmaker\ ( rmdir /s /q Matchmaker )
if exist SignallingWebserver\ ( rmdir /s /q SignallingWebserver )
if exist SFU\ ( rmdir /s /q SFU )

@Rem Rename the extracted, versioned, directory
for /d %%i in ("PixelStreamingInfrastructure-*") do (
  for /d %%j in ("%%i/*") do (
    echo "%%i\%%j"
    move "%%i\%%j" .
  )
  for %%j in ("%%i/*") do (
    echo "%%i\%%j"
    move "%%i\%%j" .
  )

  echo "%%i"
  rmdir /s /q "%%i"
)

@Rem Delete the downloaded zip
del ps-infra.zip

@Rem Create a DOWNLOAD_VERSION file, which we use as a comparison file to check if we should auto upgrade when these scripts are run again
echo %DownloadVersion%> DOWNLOAD_VERSION
goto :EOF

:print_help
echo.
echo  Tool for fetching PixelStreaming Infrastructure. If no flags are set specifying a version to fetch,
echo  the recommended version will be chosen as a default.
echo.
echo  Usage:
echo    %~n0%~x0 [^/h] [^/v ^<UE version^>] [^/b ^<branch^>] [^/t ^<tag^>] [^/r ^<release^>]
echo  Where:
echo    /v      Specify a version of Unreal Engine to download the recommended release for
echo    /b      Specify a specific branch for the tool to download from repo
echo    /t      Specify a specific tag for the tool to download from repo
echo    /r      Specify a specific release url path e.g. https://github.com/EpicGames/PixelStreamingInfrastructure/releases/download/${RELEASE_HERE}.zip
echo    /h      Display this help message
goto :EOF