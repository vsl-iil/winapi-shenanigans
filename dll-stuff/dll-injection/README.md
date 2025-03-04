DLL-инъекции
============

Загрузка сторонней DLL-библиотеки в процесс с целью выполнения кода, не 
предусмотренного разработчиком изначальной программы.
Типичная последовательность действий при DLL-инъекции:

1. найти PID процесса;

2. Получить дескриптор процесса с помощью `OpenProcess`;

3. С помощью `VirtualAllocEx` выделить в процессе память;

4. Подгрузить DLL с диска или из памяти (см. ниже);

5. Запустить код DLL в отдельном потоке с помощью `CreateRemoteThread`.


### Типы DLL-инъекций

* обычная - загружает DLL с диска в удалённый процесс при помощи LoadLibrary и 
  запускает в отдельном потоке.

* самовозвратная (reflective) - в отличие от обычной, загружает DLL из памяти, а
  а не с диска.

Существуют также, например, специфические для .NET техники: 
[через сборщик мусора](https://www.ired.team/offensive-security/code-injection-process-injection/injecting-dll-via-custom-.net-garbage-collector-environment-variable-complus_gcname) и 
[инъекция C# .NET сборок](https://www.ired.team/offensive-security/code-injection-process-injection/injecting-and-executing-.net-assemblies-to-unmanaged-process).

### Что дальше

Логическим продолжением темы инъекций и Code/Process Execution являются шеллкоды
и [хукинг](../hooking/README.md) (см. соседнюю директорию этого репозитория).