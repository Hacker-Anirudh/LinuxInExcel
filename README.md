# Linux In Excel
Linux running in Excel
![image](https://github.com/user-attachments/assets/321199ec-0191-4296-815e-8da17d9454a8)

# How?
It makes use of [mini-rv32ima](https://github.com/cnlohr/mini-rv32ima). 
The emulator is built as a seperate dll which is loaded by the VBA macro. The VBA macro calls the emulator in the dll and gets the output and writes it into the cells in the spreadsheet.
The thing is evidently very buggy but I did not want to spend a lot of time on it. This was done mostly for fun. Evidently this was cheating since I did not rewrite the emulator in VBA or Excel formulas but again I wanted to run Linux in Excel and this was one way to do so.

# Building and Running
I was using MSVC to build the dll.
Use the following command
```
cl dllmain.c /LD /Fefun.dll
```
Then in the Excel file change the path to the dll.
You can pass in input by writing the text in the C2 cell. (Thank you Endermanch for adding support for this!)
