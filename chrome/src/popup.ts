window.onload = async () => {
  const roomIdInput = document.getElementById("room-id");
  const changeButton = document.getElementById("change-button");
  const removeButton = document.getElementById("remove-button");
  if (roomIdInput === null || changeButton === null || removeButton === null) {
    return;
  }
  if (!(roomIdInput instanceof HTMLInputElement)) {
    return;
  }

  const result = await chrome.storage.local.get(["ROOM_ID_KEY"]);
  if ("ROOM_ID_KEY" in result) {
    roomIdInput.value = String(result.ROOM_ID_KEY);
  }

  changeButton.onclick = async () => {
    const roomId = roomIdInput.value;
    if (roomId === "") return;
    await chrome.storage.local.set({ ROOM_ID_KEY: roomId });
  };

  removeButton.onclick = async () => {
    await chrome.storage.local.remove("ROOM_ID_KEY");
    roomIdInput.value = "";
  };
};
