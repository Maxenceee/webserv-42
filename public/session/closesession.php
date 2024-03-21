<?php
session_start();
unset($_SESSION);
session_destroy();
?>
<html>
<head>
	<link rel="stylesheet" href="/style/style.css">
</head>
<body>
	<section class="notFound">
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
				echo 'Session is now closed!';
				?>
				<br>
				<br>
				<br>
			</div>
		</div>
	</section>
</body>
</html>