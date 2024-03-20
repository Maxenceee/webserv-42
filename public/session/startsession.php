<?php
if (session_status() == PHP_SESSION_NONE) {
	session_start();
	$_SESSION["newsession"] = "newsession";
}
?>
<html>
<head></head>
<body>
	<section class="notFound">
		<link rel="stylesheet" href="/style/style.css">
		<div class="text">
			<h1>
				<a href="/">HomePage</a>
			</h1>
			<h3>WEBSERV 1.0</h3>
			<div class="img">
				<img src="http://i.stack.imgur.com/SBv4T.gif" alt="this slowpoke moves"  width="250" />
				<br>
				<br>
				<br>
				<?php
				echo 'Session is now start!';
				?>
				<br>
				<br>
				<br>
			</div>
		</div>
	</section>
</body>
</html>