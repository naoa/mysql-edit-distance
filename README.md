# MySQL edit distance UDF

MySQLに編集距離を求める関数を追加します。

* ``edit_distance``

### ``edit_distance``関数

transposition対応のDamerau Edit distanceで編集距離を返します。

第3引数に1を指定すると、ビットパラレル法で高速に編集距離を求めます。
ビットパラレル法１バイト文字64個まで対応しています。
ビットパラレル法は日本語に対応していません。

| arg        | description |default|
|:-----------|:------------|:------|
| 1      | 文字列1| NULL |
| 2     | 文字列2 | NULL |
| 3     | ビットパラレルを使うかどうか | NULL |


### 使用例

```
CREATE TABLE IF NOT EXISTS `test` (
  `name` varchar(64) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO test VALUES("MySQL");
INSERT INTO test VALUES("MySQL2");
INSERT INTO test VALUES("MySQM");
INSERT INTO test VALUES("MySQ");
INSERT INTO test VALUES("MySLQ");
INSERT INTO test VALUES("MySOM");
INSERT INTO test VALUES("PostgreSQL");
INSERT INTO test VALUES("Oracle");

SELECT name, edit_distance(name, "MySQL") FROM test
WHERE edit_distance(name, "MySQL") < 3
ORDER BY edit_distance(name, "MySQL") ASC;

mysql> SELECT name, edit_distance(name, "MySQL") FROM test
    -> WHERE edit_distance(name, "MySQL") < 3
    -> ORDER BY edit_distance(name, "MySQL") ASC;
+--------+------------------------------+
| name   | edit_distance(name, "MySQL") |
+--------+------------------------------+
| MySQL  | 0                            |
| MySQL2 | 1                            |
| MySQ   | 1                            |
| MySQM  | 1                            |
| MySLQ  | 1                            |
| MySOM  | 2                            |
+--------+------------------------------+
6 rows in set (0.00 sec)
```

### 比較

10万レコードの比較

|文字数x文字数| 動的計画法|ビットパラレル固定長|
|--- |--------------- |---------------|
| 62x62 | 11.6524 sec | 0.1811 sec |

## Install

```bash
make
make install
mysql -ppassword < install.sql
```

デフォルトでは/usr/local/mysql/lib/plugin/edit_distance.soにインストールされます。
インストール場所を変更する場合は適宜Makefileを書き換えてください。

## Uninstall

```bash
mysql -ppassword < uninstall.sql
make uninstall
```

## Author

Naoya Murakami naoya@createfield.com

## License

Public domain.
