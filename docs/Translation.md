# Translation

GenHotkeys supports Qt's implementation of translations. If you do not have installed Qt 5, feel free to use `Resource\Translations\Tools` tools to edit *.ts files and compile them to the *.qm files.

## How to use `Qt Linguist`



## How to use `lrelease`

It is a simple CLI tool that compiles *.ts files to the *.qm files. To use this tool run command:

```cmd
lrelease.exe your-file.ts
```

In result it generates you `your-file.qm` file that could be used as editor translation, if *.qm file would be putted into `Resources\Translation` folder.

## Suggested workflow


