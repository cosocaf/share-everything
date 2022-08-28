/* eslint-disable @typescript-eslint/no-unsafe-member-access */
/* eslint-disable @typescript-eslint/no-unsafe-assignment */
// ↑型定義めんどいからとりま設定、後でちゃんとしようね

const ORIGIN = "https://share-everything-api.cosocaf.com";
// const ORIGIN = "http://localhost:8080";

const processCopy = async (
  info: chrome.contextMenus.OnClickData,
  tab: chrome.tabs.Tab,
  origin: string,
) => {
  const storage = await chrome.storage.local.get(["ROOM_ID_KEY"]);
  if (!("ROOM_ID_KEY" in storage)) {
    if (tab.id) {
      const dialog = document.getElementsByTagName(
        "share-everything-content-dialog",
      )[0];
      if (!(dialog instanceof HTMLElement)) return;
      dialog.style.visibility = "visible";
    }
    return;
  }
  if (!("ROOM_ID_KEY" in storage)) return;
  const id = storage.ROOM_ID_KEY as string;

  let content: string;
  let type = "text";
  let format = "raw";
  if (info.mediaType && info.srcUrl) {
    const media = await fetch(info.srcUrl);
    const data = await media.blob();
    content = await new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = () => {
        if (reader.result == null) {
          reject("Result is null.");
          return;
        }

        const dataUrl = reader.result.toString();
        // Data URLは"data:*/*;base64,"ではじまる。
        resolve(dataUrl.slice(dataUrl.indexOf(",") + 1));
      };
      reader.onerror = reject;
      reader.readAsDataURL(data);
    });
    type = "url";
    format = "base64";
  } else if (info.linkUrl) {
    content = info.linkUrl;
  } else if (info.selectionText) {
    content = info.selectionText;
  } else {
    return;
  }

  // TODO: ディレクトリトラバーサルとか大丈夫？
  await fetch(`${origin}/rooms/${id}`, {
    method: "PUT",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ content, type, format }),
  });
};

const processClipboard = async (
  _: chrome.contextMenus.OnClickData,
  tab: chrome.tabs.Tab,
  origin: string,
) => {
  const storage = await chrome.storage.local.get(["ROOM_ID_KEY"]);
  if (!("ROOM_ID_KEY" in storage)) {
    if (tab.id) {
      const dialog = document.getElementsByTagName(
        "share-everything-content-dialog",
      )[0];
      if (!(dialog instanceof HTMLElement)) return;
      dialog.style.visibility = "visible";
    }
    return;
  }
  if (!("ROOM_ID_KEY" in storage)) return;
  const id = storage.ROOM_ID_KEY as string;

  const response = await fetch(`${origin}/rooms/${id}`, {
    method: "GET",
  });
  const json = await response.json();

  if (json.status === "error") {
    alert("情報の取得に失敗しました。");
    console.error(json.reason);
    return;
  }

  const content = String(json.result.content);

  navigator.clipboard
    .writeText(content)
    .then(() => alert("コピーしました。"))
    .catch(() => alert("コピーできませんでした。"));
};

const onTabLoaded = async (origin: string) => {
  const dialog = document.createElement("share-everything-content-dialog");
  const shadow = dialog.attachShadow({
    mode: "closed",
  });

  const contentUrl = chrome.runtime.getURL("content.html");
  const styleUrl = chrome.runtime.getURL("content.css");

  const response = await fetch(contentUrl);
  const contentText = await response.text();

  shadow.innerHTML = `
    <link rel="stylesheet" href="${styleUrl}">
    ${contentText}
  `;

  dialog.style.visibility = "hidden";

  const content = shadow.getElementById("content");
  const roomIdInput = shadow.getElementById("room-id");
  const submitButton = shadow.getElementById("submit");
  const newButton = shadow.getElementById("create-new");

  if (
    content === null ||
    roomIdInput === null ||
    submitButton === null ||
    newButton === null
  ) {
    return;
  }
  if (!(roomIdInput instanceof HTMLInputElement)) {
    return;
  }

  content.onclick = (e) => {
    e.stopPropagation();
  };
  submitButton.onclick = async () => {
    const roomId = roomIdInput.value;
    if (roomId === "") return;
    await chrome.storage.local.set({ ROOM_ID_KEY: roomId });
    dialog.style.visibility = "hidden";
  };
  newButton.onclick = async () => {
    const response = await fetch(`${origin}/rooms`, {
      method: "POST",
    });
    const json = await response.json();
    if (json.status === "error") {
      console.error(json.reason);
      return;
    }
    // TODO: ↓as使わない
    const roomId = json.result.id as string;
    await chrome.storage.local.set({ ROOM_ID_KEY: roomId });
    dialog.style.visibility = "hidden";
    alert("ルームIDを発行しました: " + roomId);
  };

  document.body.appendChild(dialog);
  window.addEventListener("mousemove", (e) => {
    if (dialog.style.visibility === "hidden") {
      content.style.left = `${e.clientX}px`;
      content.style.top = `${e.clientY}px`;
    }
  });
  window.addEventListener("click", () => {
    dialog.style.visibility = "hidden";
  });
};

chrome.tabs.onUpdated.addListener((tabId, changeInfo) => {
  if (changeInfo.status !== "complete") return;

  chrome.scripting
    .executeScript({ target: { tabId }, func: onTabLoaded, args: [ORIGIN] })
    .catch(console.error);
});

chrome.runtime.onInstalled.addListener(() => {
  chrome.contextMenus.create({
    id: "share-everything-share",
    title: "共有",
    type: "normal",
    contexts: ["all"],
  });
  chrome.contextMenus.create({
    id: "share-everything-clipboard",
    title: "クリップボードにコピー",
    type: "normal",
    contexts: ["all"],
  });
});

chrome.contextMenus.onClicked.addListener((info, tab) => {
  switch (info.menuItemId) {
    case "share-everything-share":
      if (tab?.id) {
        chrome.scripting
          .executeScript({
            target: { tabId: tab.id },
            func: processCopy,
            args: [info, tab, ORIGIN],
          })
          .catch(console.error);
      }
      break;
    case "share-everything-clipboard":
      if (tab?.id) {
        chrome.scripting
          .executeScript({
            target: { tabId: tab.id },
            func: processClipboard,
            args: [info, tab, ORIGIN],
          })
          .catch(console.error);
      }
  }
});
