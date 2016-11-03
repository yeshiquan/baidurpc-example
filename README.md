#### A add service with nshead + mcpack using baidu-rpc

#### How to build

```bash
../../../mcpack2pb/idl2proto adder.idl > adder.proto
comake2 -UB -J8 -j8
comake2 -P
make -sj8 
```

#### Start server
```bash
./echo_server
```

#### Test 
```bash
cd php_client
~/bdp/bin/php test.php
```
