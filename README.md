# x86_Optimizing
<br>
- 목표 : Dummy Code가 적용된 x86 바이너리에 대해서 '최적화'를 수행한다.<br>
<br>
- 프로젝트 진행 동기 : 난독화 해제 관련 논문 및 연구들을 Search한 결과 난독화가 적용된 바이너리 코드에 대해서 '원본' 코드로 '복원'하는 것은 불가능하다는 결론이 도출되었으며, 원본 코드와 다르더라도 실행 결과가 동일한 코드로 최적화를 하는 방법으로 관점을 바꾸게 됨.<br>
즉, 실행 결과에 영향을 미치지 않는 명령어들을 제거함으로써 최적화를 수행하는 프로젝트<br>
<br>
- 적용 대상 : 가장 많이 사용하는 프로텍터인 VMProtect, Themida를 주 목표로 진행한다. 우선 VMProtector의 Mutation 및 Themida의 Mutation 적용 시 생성되는 Dummy Code를 최적화 하는 것을 목표로 한다.<br>
<br>
- 해당 프로젝트에서 사용할 최적화 기법<br><br>
    (1) Dead Store Elimination<br>
    (2) Constant Propagation / Register Propagation<br>
    (3) Constant Folding<br>
<br>
- 연구 계획<br>
    (1) 코드 최적화에 필요한 컴파일러 이론<br>
    (2) x86 어셈블리 코드에 대한 중간 표현 생성<br>
    (3) 중간 표현을 이용하여 코드 최적화<br>
    (4) Case Study (Themida, VMProtect의 Mutation 옵션이 적용된 코드에 대해서 테스트 진행)<br>
<br>
- 예시 <br>

(1) Dead Store Code가 포함된 x86 코드<br>
![image](https://user-images.githubusercontent.com/15829327/236723085-104c7817-03c2-452c-afa5-fcf1dfa7fe31.png)
<br>

(2) x86 코드에 대한 중간표현 생성<br>
![image](https://user-images.githubusercontent.com/15829327/236723160-22374843-4206-44ce-b431-5446805cce70.png)
<br>

(3) 최적화된 중간표현<br>
![image](https://user-images.githubusercontent.com/15829327/236723268-f46f354f-7d55-44dc-ba15-f5b3ce3512b9.png)
<br>
