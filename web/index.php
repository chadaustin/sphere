<?
  include("utility.inc.php");
  start();
?>

        coming soon i guess. (<a href="games/">game list</a>, temporary until i get the database system working)<br /><a href="http://sourceforge.net/projects/sphere">sphere project site</a><br />

<?
  mconnect();
  $QUERY = "SELECT * FROM NEWS_ENTRIES
              WHERE PARENT = 0
	      ORDER BY ID DESC";
  $RESULT = mysql_db_query($__DATABASE, $QUERY);
  while($R = mysql_fetch_array($RESULT)){
    news_item($R['ID'], $R['TOPIC'], $R['DATE'], $R['AID'], $R['ICON'], $R['BODY']);
  }

  important_links();

  conclude();
?>
