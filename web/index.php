<?
  include("utility.inc.php");
  start();
?>

  <table class="bodybody">
    <tr>
      <td class="bodybody">
        coming soon i guess. (<a href="games/">game list</a>, temporary until i get the database system working)<br /><a href="http://sourceforge.net/projects/sphere">sphere project site</a><br />

<?
  mconnect();
  $QUERY = "SELECT * FROM NEWS_ENTRIES
              WHERE PARENT = 0";
  $RESULT = mysql_db_query($__DATABASE, $QUERY);
  while($R = mysql_fetch_array($RESULT)){
    news_item($R['ID'], $R['TOPIC'], $R['DATE'], $R['AID'], $R['ICON'], $R['BODY']);
  }

  important_links();
?>

      </td>
      <td class="sidebar">
  
        <table class="sidebaritem">
          <tr>
            <td class="sidebaritemhead">
              news navigation
            </td>
          </tr>
          <tr>
            <td class="sidebaritembody">
              20.10.01 - Final Example<br />
              19.10.01 - More examples<br />
              18.10.01 - Example News<br />
              17.10.01 - Examples<br />
              16.10.01 - Example<br />
              <br />
              <div style="text-align: right; width: 100%">older ...</div>
            </td>
          </tr>
        </table>
  
        <table class="sidebaritem">
          <tr>
            <td class="sidebaritemhead">
              latest additions
            </td>
          </tr>
          <tr>
            <td class="sidebaritembody">
              20.10.01 - Games - Some dumb game<br />
              19.10.01 - Articles - Some dumb article<br />
              <br />
              <div style="text-align: right; width: 100%">complete ...</div>
            </td>
          </tr>
        </table>
  
        <table class="sidebaritem">
          <tr>
            <td class="sidebaritemhead">
              my account
            </td>
          </tr>
          <tr>
            <td class="sidebaritembody">
              currently, since i don't have access to php and don't really wanna waste my time making an independent file-driven database myself, i don't have a user system. but, when it is implemented, if you're logged in, here you'll be able to edit all sorts of things like which panels on the side you want, possibly what theme you want the site in, if you want to set up a project and what projects you are part of etc. more on this to come hopefully.
            </td>
          </tr>
        </table>

      </td>
    </tr>
  </table>

<?
  conclude();
?>