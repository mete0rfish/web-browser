import { createContext, useState } from "react";

export const HtmlContext = createContext({
  content: "",
  setContent: (content: string) => {},
});
// 초기 Context 청사진 설정

export const HtmlContextProvider = ({ children }) => {
  const [content, setContent] = useState("");

  return (
    <HtmlContext.Provider value={{ content, setContent }}>
      {/* Context 초기값 설정 */}
      {children}
    </HtmlContext.Provider>
  );
};
