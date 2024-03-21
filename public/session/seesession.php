<?php
session_start();
echo "<h3> PHP List All Session Variables</h3>";
foreach ($_SESSION as $key=>$val)
echo $key." ".$val."<br/>";
?>
<html>
<head>
	<link rel="stylesheet" href="/style/style.css">
</head>
<body>
	<section class="notFound">
		<div>
			<h3>
				<a href="/">Back HomePage</a>
			</h3>
		</div> 
	</section>
</body>
</html>