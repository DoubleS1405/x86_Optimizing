# x86_Optimizing
- Project Objective : Perform "optimization" on the x86 binary with the dummy code applied.<br>
<br>

# About the project
- Motivation for the project <br>
Searching for papers and studies on de-obfuscation led to the conclusion that it is impossible to "restore" binary code to the "original" code after it has been de-obfuscated, and shifted the perspective to optimizing the code so that the execution result is the same even if it is different from the original code.
<br>

- What do we apply x86 code optimizations to?<br>
The goal is to optimize the dummy code generated when applying VMProtector's mutation.<br>
<br>

- What optimization techniques to use? <br>
    (1) Dead Store Elimination<br>
    (2) Constant Propagation / Register Propagation<br>
    (3) Constant Folding<br>
<br>

- Project planning<br>
    (1) Researching optimization techniques to use in this project<br>
    (2) Generate an intermediate representation for x86 assembly code<br>
    (3) Optimize for intermediate representations<br>
<br>

- Example<br>
(1) intermediate representation for x86 assembly code<br>
![image](https://user-images.githubusercontent.com/15829327/236723160-22374843-4206-44ce-b431-5446805cce70.png)<br><br>

(2) Dead Store Code<br>
![image](https://github.com/DoubleS1405/x86_Optimizing/assets/15829327/ad1eb2fd-44e9-40b7-bb43-a50554cd8efb)<br><br>

(3) Optimized intermediate representation <br>
![image](https://github.com/DoubleS1405/x86_Optimizing/assets/15829327/691a1703-4d8c-40d1-88f4-3089e73827e0)<br><br>

<br>
