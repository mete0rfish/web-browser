import { createRoot } from "react-dom/client";
import App from "./App";
import "../index.css";
import { HtmlContextProvider } from "./store/HtmlStorage";

createRoot(document.getElementById("root")!).render(
  <HtmlContextProvider>
    <App />
  </HtmlContextProvider>
);
