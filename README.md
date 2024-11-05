# Socket을 이용한 검색 기능 구현 with C & React

> 목표 : 소켓프로그래밍으로 웹에 검색요청을 보내서 웹 화면으로 출력해보자

## ⚙️ 기능

1. 사용자가 검색박스에 검색을 하고 enter버튼을 클릭해서 검색을 한다.
2. c에서 짠 소켓프로그래밍 코드를 이용하여 google에 req하고 res받는다
3. 해당 res를 react를 이용하여 웹에서 뿌려준다.

### 흐름도

![image](https://github.com/user-attachments/assets/d311c087-1a59-4fad-a4a6-d08a0b60aaef)

1. React(User)가 `검색어` 혹은 `호스트네임`을 입력 후, 검색 혹은 url 버튼을 누른다.
2. Proxy(C코드)에서 React로 부터의 Request를 받아, URL의 `Path`를 보고 검색 혹은 뷰어를 구분한다.
3. Proxy에서 각 기능에 맞는 Request를 웹 페이지에 요청하여 HTML 파일을 받아온다.
4. Proxy는 받아온 리소스를 React로 응답한다.
5. React에서 리소스를 디스플레이한다.

<br/>

## ★ 사용방법

1. root 디렉토리에서 터미널에 npm i 입력한다.
2. root 디렉토리에서 npm run dev를 실행한다.
3. ./server를 이용해 컴파일된 c를 실행시켜 서버를 가동한다.
> UNIX : gcc server.c viewer.c google_search.c search_history.c -o server -lcurl -liconv
　
4. 웹 사이트 url에 http://localhost:5173/ 로 접속한다
5. 접속하면 나오는 검색창에 검색을 하고 결과를 확인한다.

<br/>

## 🔨 개선 사항

- [✔️] UI 수정
- [✔️] 해당 페이지의 이미지까지 가져올 수 없을까?
- [✔️] fetch가 최선일까? 그냥 터미널로 할까?
- [✔️] 웹에서 띄워줄 때 response message들도 보이는데 이거 어떻게 하지?
- [😡]URL을 이용한 웹 뷰 기능 동작 이상
