```
```

# setup
## <problem>Vis.javaのエラー出力を追加
```
import static java.lang.System.err;
...
err.println(N);
...
```
といったかんじ

## テストケース生成
```
javac <problem>Vis.java
./generate.sh <problem>
```

- .inと.outの二つからスコア計算できない場合は<problem>Vis.javaを頑張って書き換えて.ansを生成する
