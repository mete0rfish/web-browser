import { useState } from "react";

function App() {
  const [searchTerm, setSearchTerm] = useState("");
  const [content, setContent] = useState("");

  const handleSearch = async () => {
    try {
      // 검색어를 포함하여 서버에 요청 전송
      const response = await fetch(
        `http://localhost:8080/search=${searchTerm}`
      );
      const data = await response.text();

      // 헤더와 HTML 본문을 분리하고, 본문만 추출
      let bodyContent = data.split("\r\n\r\n")[1];

      bodyContent = bodyContent.replace(/(\r\n)?[0-9a-fA-F]+\r\n/g, ""); // 청크를 표현하는 불필요한 숫자 제거

      bodyContent = bodyContent.slice(0, -1);

      setContent(bodyContent);
    } catch (error) {
      console.error("Error fetching HTML:", error);
      setContent("Failed to fetch content."); // 에러 처리
    }
  };

  return (
    <div>
      <input
        type="text"
        value={searchTerm}
        onChange={(e) => setSearchTerm(e.target.value)}
        placeholder="Search Google"
      />
      <button onClick={handleSearch}>Search</button>
      <div dangerouslySetInnerHTML={{ __html: content }} />
    </div>
  );
}

export default App;
