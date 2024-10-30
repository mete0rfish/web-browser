import InnerHTML from "dangerously-set-html-content";
import { useContext } from "react";
import { HtmlContext } from "./store/HtmlStorage";
import SearchBox from "./components/SearchBox";

function App() {
  const { content } = useContext(HtmlContext);

  return (
    <>
      {content ? (
        <>
          <InnerHTML html={content} allowRerender={true} />
          <SearchBox />
        </>
      ) : (
        <div className="w-screen h-screen flex items-center justify-center">
          <SearchBox />
        </div>
      )}
    </>
  );
}

export default App;
