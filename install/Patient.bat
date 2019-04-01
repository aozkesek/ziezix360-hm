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

set CLASSPATH=lib\Patient.jar;lib\spring-boot-starter-web-2.1.3.RELEASE.jar;lib\spring-boot-starter-webflux-2.1.3.RELEASE.jar;lib\spring-boot-starter-security-2.1.3.RELEASE.jar;lib\spring-boot-starter-thymeleaf-2.1.3.RELEASE.jar;lib\thymeleaf-spring5-3.0.11.RELEASE.jar;lib\thymeleaf-layout-dialect-2.3.0.jar;lib\thymeleaf-extras-java8time-3.0.3.RELEASE.jar;lib\thymeleaf-expression-processor-1.1.3.jar;lib\thymeleaf-3.0.11.RELEASE.jar;lib\thymeleaf-extras-springsecurity5-3.0.4.RELEASE.jar;lib\javax.servlet-api-4.0.1.jar;lib\spring-boot-starter-json-2.1.3.RELEASE.jar;lib\spring-boot-starter-2.1.3.RELEASE.jar;lib\snakeyaml-1.23.jar;lib\hsqldb-2.4.1.jar;lib\Dao-0.0.8.jar;lib\hibernate-core-5.4.1.Final.jar;lib\guava-27.0.1-jre.jar;lib\spring-boot-starter-tomcat-2.1.3.RELEASE.jar;lib\hibernate-validator-6.0.14.Final.jar;lib\spring-webmvc-5.1.5.RELEASE.jar;lib\spring-webflux-5.1.5.RELEASE.jar;lib\spring-security-web-5.1.4.RELEASE.jar;lib\spring-web-5.1.5.RELEASE.jar;lib\spring-boot-starter-reactor-netty-2.1.3.RELEASE.jar;lib\nio-multipart-parser-1.1.0.jar;lib\spring-security-config-5.1.4.RELEASE.jar;lib\spring-boot-autoconfigure-2.1.3.RELEASE.jar;lib\spring-boot-2.1.3.RELEASE.jar;lib\spring-security-core-5.1.4.RELEASE.jar;lib\spring-context-5.1.5.RELEASE.jar;lib\spring-aop-5.1.5.RELEASE.jar;lib\ognl-3.1.12.jar;lib\attoparser-2.0.5.RELEASE.jar;lib\unbescape-1.1.6.RELEASE.jar;lib\nio-stream-storage-1.1.3.jar;lib\spring-boot-starter-logging-2.1.3.RELEASE.jar;lib\logback-classic-1.2.3.jar;lib\log4j-to-slf4j-2.11.2.jar;lib\jul-to-slf4j-1.7.25.jar;lib\slf4j-api-1.7.25.jar;lib\groovy-2.4.13.jar;lib\hibernate-commons-annotations-5.1.0.Final.jar;lib\jboss-logging-3.3.2.Final.jar;lib\javax.persistence-api-2.2.jar;lib\javassist-3.24.0-GA.jar;lib\byte-buddy-1.9.5.jar;lib\antlr-2.7.7.jar;lib\jboss-transaction-api_1.2_spec-1.1.1.Final.jar;lib\jandex-2.0.5.Final.jar;lib\classmate-1.3.4.jar;lib\jaxb-runtime-2.3.1.jar;lib\jaxb-api-2.3.1.jar;lib\javax.activation-api-1.2.0.jar;lib\dom4j-2.1.1.jar;lib\spring-orm-5.1.5.RELEASE.jar;lib\failureaccess-1.0.1.jar;lib\listenablefuture-9999.0-empty-to-avoid-conflict-with-guava.jar;lib\jsr305-3.0.2.jar;lib\checker-qual-2.5.2.jar;lib\error_prone_annotations-2.2.0.jar;lib\j2objc-annotations-1.1.jar;lib\animal-sniffer-annotations-1.17.jar;lib\javax.annotation-api-1.3.2.jar;lib\spring-jdbc-5.1.5.RELEASE.jar;lib\spring-tx-5.1.5.RELEASE.jar;lib\spring-beans-5.1.5.RELEASE.jar;lib\spring-expression-5.1.5.RELEASE.jar;lib\spring-core-5.1.5.RELEASE.jar;lib\jackson-datatype-jdk8-2.9.8.jar;lib\jackson-datatype-jsr310-2.9.8.jar;lib\jackson-module-parameter-names-2.9.8.jar;lib\jackson-databind-2.9.8.jar;lib\tomcat-embed-websocket-9.0.16.jar;lib\tomcat-embed-core-9.0.16.jar;lib\tomcat-embed-el-9.0.16.jar;lib\validation-api-2.0.1.Final.jar;lib\reactor-netty-0.8.5.RELEASE.jar;lib\reactor-core-3.2.6.RELEASE.jar;lib\txw2-2.3.1.jar;lib\istack-commons-runtime-3.0.7.jar;lib\stax-ex-1.8.jar;lib\FastInfoset-1.2.15.jar;lib\spring-jcl-5.1.5.RELEASE.jar;lib\jackson-annotations-2.9.0.jar;lib\jackson-core-2.9.8.jar;lib\netty-codec-http2-4.1.33.Final.jar;lib\netty-handler-proxy-4.1.33.Final.jar;lib\netty-codec-http-4.1.33.Final.jar;lib\netty-handler-4.1.33.Final.jar;lib\netty-transport-native-epoll-4.1.33.Final-linux-x86_64.jar;lib\reactive-streams-1.0.2.jar;lib\logback-core-1.2.3.jar;lib\log4j-api-2.11.2.jar;lib\tomcat-annotations-api-9.0.16.jar;lib\netty-codec-socks-4.1.33.Final.jar;lib\netty-codec-4.1.33.Final.jar;lib\netty-transport-native-unix-common-4.1.33.Final.jar;lib\netty-transport-4.1.33.Final.jar;lib\netty-buffer-4.1.33.Final.jar;lib\netty-resolver-4.1.33.Final.jar;lib\netty-common-4.1.33.Final.jar

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
