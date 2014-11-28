$makeNsis = 'C:\Program Files\NSIS\makensis.exe'
$myDir = (get-item $($myinvocation.InvocationName)).DirectoryName
$kflogProjectDir = "$myDir\..\kflog"
$rootDevDir = (get-item $mydir).Parent.Parent.FullName
$nsisScript = "$myDir/setup.nsi"
$zipInstallFile = "$rootDevDir/PortableKFLog.zip"
$exeInstallFile = "$rootDevDir/KflogSetup.exe"
"zipInstallFile: $zipInstallFile"
"exeInstallFile: $exeInstallFile"

"hier: $rootDevDir"

$QtFiles = "QtCore4.dll",
           "QtGui4.dll",
           "QtNetwork4.dll",
           "QtOpenGL4.dll",
           "QtXml4.dll"
$KFlogLanguageFiles = "kflog"
$QtLanguageFiles = "qt"
$Languages = "de"

$MinGwFiles = "libgcc_s_dw2-1.dll",
              "..\mingw32\opt\bin\libwinpthread-1.dll",
              "libstdc++-6.dll"
$KflogFiles = "Release\kflog.exe"
$LicenseFiles = "$myDir\License.rtf",
                "$myDir\license_beta.rtf",
                "$myDir\RemoveKFLogDataDirectories.cmd",
                "$myDir\RemoveKFLogRegistryKey.cmd"


if ($env:QT_DIR -eq $NULL) 
{
    $QtDirectory = "C:\Qt\4.8.6"
}
else
{
    $QtDirectory = $env:QT_DIR
}

# MinGw directory
$gccObject = (get-item (Get-Command gcc).Definition)
$mingwDirectory = $gccObject.DirectoryName

if ($args.Length -eq 0)
{
    $MakeDirectory = "C:/Users/peter/dev/KFLog/build-windows_setup-Desktop-Release"
}
else
{
    $MakeDirectory = $args
}

$kflogMakeDirectory = "$MakeDirectory/release"
# C:\Users\peter\dev\build-kflog-Desktop-Release\kflog\release

"Creating language files"
Foreach ($KFLogLanguageFile in $KFlogLanguageFiles)
{
   Foreach ($Language in $Languages)
   {
        & "$QtDirectory/bin/lrelease.exe" "$kflogProjectDir/$($KFLogLanguageFile)_$Language.ts"
   }
}


# create working directory if not existing
$workDirectory = "$makeDirectory/tmp"
if (-not (test-path $workDirectory))
{
    New-Item -ItemType Directory $workDirectory
}

Remove-Item -Recurse -Force $workDirectory/*
New-Item -ItemType Directory $workDirectory/translations

# copy the files to the working directory
$QtFiles | %{Copy-Item -Verbose $QtDirectory/bin/$_ $workDirectory}
$MinGwFiles | %{Copy-Item -Verbose $MinGwDirectory/$_ $workDirectory}
$KflogFiles | %{Copy-Item -Verbose $MakeDirectory/$_ $workDirectory}
$LicenseFiles | %{Copy-Item -Verbose $_ $workDirectory}


"Copying language files..."
Foreach ($QtLanguageFile in $QtLanguageFiles)
{
    Foreach ($Language in $Languages)
    {
        Copy-Item -v "$QtDirectory/translations/$($QtLanguageFile)_$Language.qm" "$workDirectory/translations/"
    }
}
Foreach ($KFLogLanguageFile in $KFlogLanguageFiles)
{
   Foreach ($Language in $Languages)
   {
        Copy-Item -v "$kflogProjectDir/$($KFLogLanguageFile)_$Language.qm" "$workDirectory/translations"
   }
}

$zipfile = "$MakeDirectory/../PortableKFLog.zip"
if ((test-path $zipfile))
{
    Remove-Item $zipfile
}

# Load the compression namespace 
# and yes, I know this usage is obsolete - but it works. 
# Ignore the output 
[System.Reflection.Assembly]::LoadWithPartialName('System.IO.Compression.FileSystem') | Out-Null  
 
# Create a type accellerator for the zipfile class 
[System.Type] $TypeAcceleratorsType=[System.Management.Automation.PSObject].Assembly.GetType('System.Management.Automation.TypeAccelerators',$True,$True) 
$TypeAcceleratorsType::Add('Zipfile','System.IO.Compression.Zipfile') 

"zipfile: $zipfile"
"workDirectory: $workDirectory"
     
# Now create the Zip file 
Try { 
  [Zipfile]::CreateFromDirectory( $workDirectory, $zipfile) 
  "Zip file created:";ls $zipfile} 
Catch { 
  "Zip File NOT created" 
  $Error[0]} 

if ((Test-Path $zipInstallFile))
{
    Remove-Item $zipInstallFile
}

Move-Item -Verbose $zipfile $zipInstallFile

  &"$makeNSIS" "/DMingwBinPath=$MinGwDirectory" "/DQtBinPath=$QtDirectory/bin" "/DExeSourcePath=$kflogMakeDirectory" "/DKflogProjDir=$kflogProjectDir" "/NOCONFIG" "$nsisScript"

if ((Test-Path $exeInstallFile))
{
    Remove-Item $exeInstallFile
}
Move-Item -Verbose "$myDir/KflogSetup.exe" $exeInstallFile
