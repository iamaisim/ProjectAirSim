#!/bin/bash
# Copyright Epic Games, Inc. All Rights Reserved.

BASH_LOCATION=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd "${BASH_LOCATION}" > /dev/null

print_help() {
 echo "
 Tool for fetching PixelStreaming Infrastructure. If no flags are set specifying a version to fetch, 
 the recommended version will be chosen as a default.

 Usage:
   ${0} [-h] [-v <UE version>] [-b <branch>] [-t <tag>]
 Where:
   -v      Specify a version of Unreal Engine to download the recommended 
           release for
   -b      Specify a specific branch for the tool to download from repo
   -t      Specify a specific tag for the tool to download from repo
   -r      Specify a specific release url path e.g. https://github.com/EpicGames/PixelStreamingInfrastructure/releases/download/<RELEASE_VERSION>/<RELEASE_VERSION>.zip
   -h      Display this help message
"
 exit 1
}

# Set all default variables (e.g. # Name and version of ps-infra that we are downloading)
PSInfraOrg=EpicGames
PSInfraRepo=PixelStreamingInfrastructure
PSInfraTagOrBranch=UE5.2
RefType=heads
IsTag=0
ReleaseUrlBase=https://github.com/EpicGames/PixelStreamingInfrastructure/releases/download
# Unset any variables that don't have defaults that we use that may have persisted between bash terminals.
unset Url
unset DownloadVersion
unset FlagPassed
unset ReleaseVersion
unset ReleaseUrl

while(($#)) ; do
  case "$1" in
   -h ) print_help;;
   -v ) UEVersion="$2"; FlagPassed=1; shift 2;;
   -b ) PSInfraTagOrBranch="$2"; FlagPassed=1; IsTag=0; shift 2;;
   -t ) PSInfraTagOrBranch="$2"; FlagPassed=1; IsTag=1; shift 2;;
   -r ) ReleaseVersion="$2"; FlagPassed=1; IsTag=0; ReleaseUrl=$ReleaseUrlBase/$ReleaseVersion/$ReleaseVersion.tar.gz; shift 2;;
   * ) echo "Unknown command: $1"; shift;;
  esac
 done

# If a UE version is supplied set the right branch or tag to fetch for that version of UE
if [ ! -z "$UEVersion" ]
then
  if [ "$UEVersion" = "4.26" ]
  then
    PSInfraTagOrBranch=UE4.26
    IsTag=0
  fi
  if [ "$UEVersion" = "4.27" ]
  then
    PSInfraTagOrBranch=UE4.27
    IsTag=0
  fi
  if [ "$UEVersion" = "5.0" ]
  then
    PSInfraTagOrBranch=UE5.0
    IsTag=0
  fi
  if [ "$UEVersion" = "5.1" ]
  then
    PSInfraTagOrBranch=UE5.1
    IsTag=0
  fi
  if [ "$UEVersion" = "5.2" ]
  then
    PSInfraTagOrBranch=UE5.2
    IsTag=0
  fi
fi

# If no arguments select a specific version, fetch the appropriate default
if [ -z "$PSInfraTagOrBranch" ]
then
  PSInfraTagOrBranch=UE5.2
  IsTag=0
fi
echo "Tag or branch: $PSInfraTagOrBranch"

# Whether the named reference is a tag or a branch affects the Url we fetch it on
if [ "$IsTag" -eq 1 ]
then
  RefType=tags
else
  RefType=heads
fi

# We have a branch, no user-specified release, then check repo for the presence of a RELEASE_VERSION file in the current branch.
if [ "$IsTag" -eq 0 ] && [ -z "$ReleaseUrl" ] && [ -z "$FlagPassed" ]
then
  RelUrl=https://raw.githubusercontent.com/EpicGames/PixelStreamingInfrastructure/$PSInfraTagOrBranch/RELEASE_VERSION
  if curl --output /dev/null --silent -r 0-0 --fail "$RelUrl"; then
    ReleaseVersion="$PSInfraTagOrBranch-$(curl -L -s $RelUrl)"
    ReleaseUrl=https://github.com/EpicGames/PixelStreamingInfrastructure/releases/download/$ReleaseVersion/$ReleaseVersion.tar.gz
    echo "Valid RELEASE_VERSION file found in Github repo at $RelUrl"
  else
    echo "RELEASE_VERSION file does not exist at: $RelUrl"
  fi
else
  echo "Skipping downloading RELEASE_VERSION file."
fi

#Set our DownloadVersion here as we use this to check the contents of our DOWNLOAD_VERSION file shortly.
DownloadVersion="$PSInfraTagOrBranch"
if [ ! -z "$ReleaseVersion" ] 
then
  DownloadVersion="$ReleaseVersion"
  echo "Release: $ReleaseVersion"
fi

#Rem Check for the existence of a DOWNLOAD_VERSION file and if found, check its contents against our $DownloadVersion
if test -f DOWNLOAD_VERSION;
then
  PREVIOUS_DOWNLOAD_VERSION=$(cat DOWNLOAD_VERSION)
  if [ "$DownloadVersion" = "$PREVIOUS_DOWNLOAD_VERSION" ]
  then
    echo "Downloaded version ($DownloadVersion) of PS infra matches release version ($PREVIOUS_DOWNLOAD_VERSION)...skipping install."
    exit 0
  else
    echo "There is a newer released version ($DownloadVersion) - had ($PREVIOUS_DOWNLOAD_VERSION), downloading..."
    #Remove old infra
    rm -rf Frontend
    rm -rf Matchmaker
    rm -rf SignallingWebServer
    rm -rf SFU
  fi
else 
  echo "DOWNLOAD_VERSION file not found..."
fi

# Pre-download - Set the download url to the .zip of the branch
Url=https://github.com/$PSInfraOrg/$PSInfraRepo/archive/refs/$RefType/$PSInfraTagOrBranch.tar.gz

#Check if ReleaseUrl is valid by CURLing with fail fast and if success then set it to our download url
if [ ! -z "$ReleaseUrl" ]
then
  echo "Checking if release url is valid, url: $ReleaseUrl"
  if curl --output /dev/null --silent -r 0-0 --fail "$ReleaseUrl"; then
    echo "Valid release url at: $ReleaseUrl"
    Url=$ReleaseUrl
  else
    echo "Invalid Github release url: $ReleaseUrl"
    exit 1
  fi 
fi

# Download - download ps-infra and follow redirects.
echo "Beginning ps-infra download from: $Url"
curl -L $Url > ps-infra.tar.gz

# Unarchive the .tar
tar -xmf ps-infra.tar.gz || $(echo "bad archive, contents:" && head --lines=20 ps-infra.tar.gz && exit 0)

# Move the server folders into the current directory (WebServers) and delete the original directory
mv PixelStreamingInfrastructure-*/* .
# Copy any files and folders beginning with dot (ignored by * glob) and discard errors regarding to not being able to move "." and ".."
mv PixelStreamingInfrastructure-*/.* . 2>/dev/null || :
rm -rf PixelStreamingInfrastructure-*

# Delete the downloaded tar
rm ps-infra.tar.gz

#Create a DOWNLOAD_VERSION file, which we use as a comparison file to check if we should auto upgrade when these scripts are run again
echo "$DownloadVersion" >| DOWNLOAD_VERSION
exit 0