<?

include("../utility.inc.php");

start();

global $__DATABASE;

if(isset($act)){
  if($act=="add" && isset($F_AID) && isset($F_TOPIC) && isset($F_BODY) && isset($F_ICON) && isset($F_DATE)){
    mconnect();
    $QUERY = "INSERT INTO NEWS_ENTRIES VALUES(
              0,
              \"$F_DATE\",
              $F_AID,
              $F_ICON,
              0,
              \"$F_BODY\",
              \"$F_TOPIC\");";
    $RESULT = mysql_db_query($__DATABASE, $QUERY);
    if($RESULT)
      echo "\n<div class=\"success\">added entry to news forum.</div>\n\n";
    else
      echo "\n<div class=\"error\">error. hmph.</div>\n\n";
  }
}

else {
?>

<div style="text-align: center; margin-left: auto; margin-right: auto; width: 100%">
  <table class="item" style="width: 200pt; margin-left: auto; margin-right: auto">
    <tr>
      <td class="itemhead" style="text-align: center">
        create news thread
      </td>
    </tr>
    <tr>
      <td class="itembody">
        <form id="addnews" method="post" action="http://sphere.sourceforge.net/admin/addnews.php?act=add">
          <div>
            author<br />
            <input type="text" name="F_AID" style="width: 100%" /><br />
            date<br />
            <input type="text" name="F_DATE" style="width: 100%" /><br />
            icon<br />
            <input type="text" name="F_ICON" style="width: 100%" /><br />
            topic<br />
            <input type="text" name="F_TOPIC" style="width: 100%" /><br />
            body<br />
            <textarea name="F_BODY" style="width: 100%; height:100pt" rows="5" cols="10"></textarea><br />
            <a href="javascript:document.getElementById('addnews').submit()">submit</a>
          </div>
        </form>
      </td>
    </tr>
  </table>
  <br />
</div>

<?
}

important_links();

conclude();

?>
