# x86_Optimizing
- Project Objective : Perform "optimization" on the x86 binary with the dummy code applied.<br>
<br>

# About the project
- Motivation for the project <br>
Searching for papers and studies on de-obfuscation led to the conclusion that it is impossible to "restore" binary code to the "original" code after it has been de-obfuscated, and shifted the perspective to optimizing the code so that the execution result is the same even if it is different from the original code.
<br>

- What do we apply x86 code optimizations to?<br>
가장 많이 사용하는 프로텍터인 VMProtect, Themida를 주 목표로 진행한다. 우선 VMProtector의 Mutation 및 Themida의 Mutation 적용 시 생성되는 Dummy Code를 최적화 하는 것을 목표로 한다.<br>
<br>

- What optimization techniques to use? <br>
    (1) Dead Store Elimination<br>
    (2) Constant Propagation / Register Propagation<br>
    (3) Constant Folding<br>
<br>

- Project planning<br>
    (1) Researching optimization techniques to use in your project<br>
    (2) Create an intermediate representation for x86 assembly code<br>
    (3) Optimize for intermediate representations<br>
    (4) 테스트 (Themida, VMProtect의 Mutation 옵션이 적용된 코드에 대해서 테스트 진행)<br>
<br>

- Example<br>
(1) intermediate representation for x86 assembly code<br>
![image](https://user-images.githubusercontent.com/15829327/236723160-22374843-4206-44ce-b431-5446805cce70.png)<br><br>

(2) Dead Store Code<br>
![image](https://github.com/DoubleS1405/x86_Optimizing/assets/15829327/ad1eb2fd-44e9-40b7-bb43-a50554cd8efb)<br><br>

(3) Optimized intermediate representation <br>
![image](https://github.com/DoubleS1405/x86_Optimizing/assets/15829327/691a1703-4d8c-40d1-88f4-3089e73827e0)<br><br>

<br>
