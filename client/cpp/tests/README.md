# ProjectAirSim C++ Client Tests

Este directorio tiene tests para el cliente C++.

## Tipos de tests

| Test | Target | Usa Unreal/simserver real | Para que sirve |
| --- | --- | --- | --- |
| Mocked unit test | `cpp_client_mocked_unit_tests` | No | Valida el cliente C++ con NNG mockeado (`FakeNNGI`). |
| Integration test | `cpp_client_unit_tests` | Si | Valida que el cliente C++ pueda conectarse a Unreal/ProjectAirSim real. |

## Compilar

Desde la raiz del repo:

```bash
cmake --build client/cpp/build_linux/Debug --target cpp_client_mocked_unit_tests -j16
cmake --build client/cpp/build_linux/Debug --target cpp_client_unit_tests -j16
```

## Correr test mockeado

Este test no necesita Unreal corriendo.

```bash
./client/cpp/build_linux/Debug/cpp_client_mocked_unit_tests
```

Resultado esperado:

```text
[OK] C++ client unit tests passed
```

## Correr test contra Unreal real

Primero levantar Unreal/ProjectAirSim o el simserver. Despues:

```bash
./client/cpp/build_linux/Debug/cpp_client_unit_tests
```

Si Unreal esta corriendo y el server escucha en `127.0.0.1`, deberia pasar.

Si Unreal no esta corriendo, debe fallar. Eso es esperado:

```text
[FAIL] Client::Connect(127.0.0.1) failed: ...
[INFO] Start Unreal/ProjectAirSim simserver before running this integration test.
```

## Usar otro host

El integration test acepta el host como primer argumento:

```bash
./client/cpp/build_linux/Debug/cpp_client_unit_tests 192.168.0.50
```

## Archivos

| Archivo | Descripcion |
| --- | --- |
| `CppClientUnitTests.cpp` | Tests unitarios con NNG mockeado. |
| `CppClientIntegrationTests.cpp` | Test de conexion real contra Unreal/ProjectAirSim. |
| `FakeNNGI.h` / `FakeNNGI.cpp` | Implementacion fake de NNG para los unit tests. |

