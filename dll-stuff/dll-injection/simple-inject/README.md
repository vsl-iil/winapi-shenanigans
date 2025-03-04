Компиляция
----------

Компилируем DLL:

```powershell
clang evil.cpp -shared -o evil.dll
```

Компилируем программу-инжектор:

```powershell
clang injector.cpp -o injector.exe
```

Инжектим DLL в процесс notepad.exe:

```powershell
.\injector.exe notepad.exe evil.dll
```

Откроется окно с сообщением, вызванное загруженным DLL.