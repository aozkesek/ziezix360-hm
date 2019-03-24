@if "%DEBUG%" == "" @echo off
@rem ##########################################################################
@rem
@rem  Patient startup script for Windows
@rem
@rem ##########################################################################

@rem Set local scope for the variables with windows NT shell
if "%OS%"=="Windows_NT" setlocal

set DIRNAME=%~dp0
if "%DIRNAME%" == "" set DIRNAME=.
set APP_BASE_NAME=%~n0
set APP_HOME=%DIRNAME%..

@rem Add default JVM options here. You can also use JAVA_OPTS and PATIENT_OPTS to pass JVM options to this script.
set DEFAULT_JVM_OPTS=

@rem Find java.exe
if defined JAVA_HOME goto findJavaFromJavaHome

set JAVA_EXE=java.exe
%JAVA_EXE% -version >NUL 2>&1
if "%ERRORLEVEL%" == "0" goto init

echo.
echo ERROR: JAVA_HOME is not set and no 'java' command could be found in your PATH.
echo.
echo Please set the JAVA_HOME variable in your environment to match the
echo location of your Java installation.

goto fail

:findJavaFromJavaHome
set JAVA_HOME=%JAVA_HOME:"=%
set JAVA_EXE=%JAVA_HOME%/bin/java.exe

if exist "%JAVA_EXE%" goto init

echo.
echo ERROR: JAVA_HOME is set to an invalid directory: %JAVA_HOME%
echo.
echo Please set the JAVA_HOME variable in your environment to match the
echo location of your Java installation.

goto fail

:init
@rem Get command-line arguments, handling Windows variants

if not "%OS%" == "Windows_NT" goto win9xME_args

:win9xME_args
@rem Slurp the command line arguments.
set CMD_LINE_ARGS=
set _SKIP=2

:win9xME_args_slurp
if "x%~1" == "x" goto execute

set CMD_LINE_ARGS=%*

:execute
@rem Setup the command line

set CLASSPATH=%APP_HOME%\lib\Patient.jar;%APP_HOME%\lib\spring-boot-starter-web-2.1.3.RELEASE.jar;%APP_HOME%\lib\spring-boot-starter-webflux-2.1.3.RELEASE.jar;%APP_HOME%\lib\spring-boot-starter-security-2.1.3.RELEASE.jar;%APP_HOME%\lib\spring-boot-starter-thymeleaf-2.1.3.RELEASE.jar;%APP_HOME%\lib\thymeleaf-spring5-3.0.11.RELEASE.jar;%APP_HOME%\lib\thymeleaf-layout-dialect-2.3.0.jar;%APP_HOME%\lib\thymeleaf-extras-java8time-3.0.3.RELEASE.jar;%APP_HOME%\lib\thymeleaf-expression-processor-1.1.3.jar;%APP_HOME%\lib\thymeleaf-3.0.11.RELEASE.jar;%APP_HOME%\lib\thymeleaf-extras-springsecurity5-3.0.4.RELEASE.jar;%APP_HOME%\lib\javax.servlet-api-4.0.1.jar;%APP_HOME%\lib\spring-boot-starter-json-2.1.3.RELEASE.jar;%APP_HOME%\lib\spring-boot-starter-2.1.3.RELEASE.jar;%APP_HOME%\lib\snakeyaml-1.23.jar;%APP_HOME%\lib\hsqldb-2.4.1.jar;%APP_HOME%\lib\Dao-0.0.8.jar;%APP_HOME%\lib\hibernate-core-5.4.1.Final.jar;%APP_HOME%\lib\guava-27.0.1-jre.jar;%APP_HOME%\lib\spring-boot-starter-tomcat-2.1.3.RELEASE.jar;%APP_HOME%\lib\hibernate-validator-6.0.14.Final.jar;%APP_HOME%\lib\spring-webmvc-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-webflux-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-security-web-5.1.4.RELEASE.jar;%APP_HOME%\lib\spring-web-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-boot-starter-reactor-netty-2.1.3.RELEASE.jar;%APP_HOME%\lib\nio-multipart-parser-1.1.0.jar;%APP_HOME%\lib\spring-security-config-5.1.4.RELEASE.jar;%APP_HOME%\lib\spring-boot-autoconfigure-2.1.3.RELEASE.jar;%APP_HOME%\lib\spring-boot-2.1.3.RELEASE.jar;%APP_HOME%\lib\spring-security-core-5.1.4.RELEASE.jar;%APP_HOME%\lib\spring-context-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-aop-5.1.5.RELEASE.jar;%APP_HOME%\lib\ognl-3.1.12.jar;%APP_HOME%\lib\attoparser-2.0.5.RELEASE.jar;%APP_HOME%\lib\unbescape-1.1.6.RELEASE.jar;%APP_HOME%\lib\nio-stream-storage-1.1.3.jar;%APP_HOME%\lib\spring-boot-starter-logging-2.1.3.RELEASE.jar;%APP_HOME%\lib\logback-classic-1.2.3.jar;%APP_HOME%\lib\log4j-to-slf4j-2.11.2.jar;%APP_HOME%\lib\jul-to-slf4j-1.7.25.jar;%APP_HOME%\lib\slf4j-api-1.7.25.jar;%APP_HOME%\lib\groovy-2.4.13.jar;%APP_HOME%\lib\hibernate-commons-annotations-5.1.0.Final.jar;%APP_HOME%\lib\jboss-logging-3.3.2.Final.jar;%APP_HOME%\lib\javax.persistence-api-2.2.jar;%APP_HOME%\lib\javassist-3.24.0-GA.jar;%APP_HOME%\lib\byte-buddy-1.9.5.jar;%APP_HOME%\lib\antlr-2.7.7.jar;%APP_HOME%\lib\jboss-transaction-api_1.2_spec-1.1.1.Final.jar;%APP_HOME%\lib\jandex-2.0.5.Final.jar;%APP_HOME%\lib\classmate-1.3.4.jar;%APP_HOME%\lib\jaxb-runtime-2.3.1.jar;%APP_HOME%\lib\jaxb-api-2.3.1.jar;%APP_HOME%\lib\javax.activation-api-1.2.0.jar;%APP_HOME%\lib\dom4j-2.1.1.jar;%APP_HOME%\lib\spring-orm-5.1.5.RELEASE.jar;%APP_HOME%\lib\failureaccess-1.0.1.jar;%APP_HOME%\lib\listenablefuture-9999.0-empty-to-avoid-conflict-with-guava.jar;%APP_HOME%\lib\jsr305-3.0.2.jar;%APP_HOME%\lib\checker-qual-2.5.2.jar;%APP_HOME%\lib\error_prone_annotations-2.2.0.jar;%APP_HOME%\lib\j2objc-annotations-1.1.jar;%APP_HOME%\lib\animal-sniffer-annotations-1.17.jar;%APP_HOME%\lib\javax.annotation-api-1.3.2.jar;%APP_HOME%\lib\spring-jdbc-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-tx-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-beans-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-expression-5.1.5.RELEASE.jar;%APP_HOME%\lib\spring-core-5.1.5.RELEASE.jar;%APP_HOME%\lib\jackson-datatype-jdk8-2.9.8.jar;%APP_HOME%\lib\jackson-datatype-jsr310-2.9.8.jar;%APP_HOME%\lib\jackson-module-parameter-names-2.9.8.jar;%APP_HOME%\lib\jackson-databind-2.9.8.jar;%APP_HOME%\lib\tomcat-embed-websocket-9.0.16.jar;%APP_HOME%\lib\tomcat-embed-core-9.0.16.jar;%APP_HOME%\lib\tomcat-embed-el-9.0.16.jar;%APP_HOME%\lib\validation-api-2.0.1.Final.jar;%APP_HOME%\lib\reactor-netty-0.8.5.RELEASE.jar;%APP_HOME%\lib\reactor-core-3.2.6.RELEASE.jar;%APP_HOME%\lib\txw2-2.3.1.jar;%APP_HOME%\lib\istack-commons-runtime-3.0.7.jar;%APP_HOME%\lib\stax-ex-1.8.jar;%APP_HOME%\lib\FastInfoset-1.2.15.jar;%APP_HOME%\lib\spring-jcl-5.1.5.RELEASE.jar;%APP_HOME%\lib\jackson-annotations-2.9.0.jar;%APP_HOME%\lib\jackson-core-2.9.8.jar;%APP_HOME%\lib\netty-codec-http2-4.1.33.Final.jar;%APP_HOME%\lib\netty-handler-proxy-4.1.33.Final.jar;%APP_HOME%\lib\netty-codec-http-4.1.33.Final.jar;%APP_HOME%\lib\netty-handler-4.1.33.Final.jar;%APP_HOME%\lib\netty-transport-native-epoll-4.1.33.Final-linux-x86_64.jar;%APP_HOME%\lib\reactive-streams-1.0.2.jar;%APP_HOME%\lib\logback-core-1.2.3.jar;%APP_HOME%\lib\log4j-api-2.11.2.jar;%APP_HOME%\lib\tomcat-annotations-api-9.0.16.jar;%APP_HOME%\lib\netty-codec-socks-4.1.33.Final.jar;%APP_HOME%\lib\netty-codec-4.1.33.Final.jar;%APP_HOME%\lib\netty-transport-native-unix-common-4.1.33.Final.jar;%APP_HOME%\lib\netty-transport-4.1.33.Final.jar;%APP_HOME%\lib\netty-buffer-4.1.33.Final.jar;%APP_HOME%\lib\netty-resolver-4.1.33.Final.jar;%APP_HOME%\lib\netty-common-4.1.33.Final.jar

@rem Execute Patient
"%JAVA_EXE%" %DEFAULT_JVM_OPTS% %JAVA_OPTS% %PATIENT_OPTS%  -classpath "%CLASSPATH%" com.ziezix360.hm.patient.PatientApp %CMD_LINE_ARGS%

:end
@rem End local scope for the variables with windows NT shell
if "%ERRORLEVEL%"=="0" goto mainEnd

:fail
rem Set variable PATIENT_EXIT_CONSOLE if you need the _script_ return code instead of
rem the _cmd.exe /c_ return code!
if  not "" == "%PATIENT_EXIT_CONSOLE%" exit 1
exit /b 1

:mainEnd
if "%OS%"=="Windows_NT" endlocal

:omega
