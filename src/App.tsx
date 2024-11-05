import InnerHTML from "dangerously-set-html-content";
import { useContext, useEffect, useState } from "react";
import { HtmlContext } from "./store/HtmlStorage";
import SearchBox from "./components/SearchBox";

function App() {
  const { content } = useContext(HtmlContext);
  const [log, setLog] = useState<string[]>([]);

  useEffect(() => {
    const fetchLog = async () => {
      try {
        const response = await fetch(`http://localhost:8080/history`);
        const data = await response.text();

        // HTTP 헤더와 본문을 빈 줄로 구분해 헤더를 제거
        const body = data.includes("\n\n") ? data.split("\n\n")[1] : data;

        // 본문을 줄 단위로 분리해 배열로 저장
        const lines = body
          .split("\n")
          .slice(4) // 인덱스 4부터 끝까지 가져옴
          .filter((line) => line !== ""); // 빈 문자열 제거

        // 로컬 스토리지에 저장
        localStorage.setItem("log", JSON.stringify(lines));
        setLog(lines); // 상태에 배열 형태로 설정
      } catch (error) {
        console.error("Error fetching log:", error);
      }
    };

    fetchLog();

    // 로컬 스토리지에서 로그를 가져와 상태에 설정
    const storedLog = JSON.parse(localStorage.getItem("log") || "[]");
    setLog(storedLog);
  }, []);

  return (
    <>
      {content ? (
        <>
          <InnerHTML html={content} allowRerender={true} />
          <SearchBox />
        </>
      ) : (
        <div className="w-screen h-screen flex flex-col items-center justify-center space-y-4 p-4">
          <SearchBox />
          <span className="text-lg font-semibold">LOG</span>
          <div className="w-full max-w-lg bg-gray-100 p-4 rounded-lg shadow-md overflow-y-auto h-96">
            {log.map((line, index) => (
              <p
                key={index}
                className="text-sm text-gray-800 mb-2 border-b border-gray-300 pb-1"
              >
                {line}
              </p>
            ))}
          </div>
        </div>
      )}
    </>
  );
}

export default App;
