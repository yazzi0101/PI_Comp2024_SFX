function toggleChat() {

    const chat =
        document.getElementById("chatBox");

    if (chat.style.display === "flex") {

        chat.style.display = "none";

    } else {

        chat.style.display = "flex";
    }
}