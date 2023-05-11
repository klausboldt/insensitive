var navbar = `
        <ul>
            <li><a href="default.html">Home</a></li>
            <li><a href="manual.html">User Manual</a></li>
            <li><a href="commandline.html">Command Line</a></li>
            <li><a href="index.html">Index</a></li>
        </ul>`;

// inserting navbar in beginning of body
let str = navigator.userAgent;
if (!str.includes("AppleWebKit") || str.includes("Macintosh") || str.includes("Windows")) {
    document.body.style.marginTop='55px';
    let replace_script = document.querySelector("script#navbar");
    let with_navbar = document.createElement("navbar");
    with_navbar.innerHTML = navbar;
    replace_script.parentNode.replaceChild(with_navbar, replace_script);
}
