<?

include_once("/home/groups/s/sp/sphere/htdocs/passwords.inc.php");
$__DATABASE = "sphere";
$__SITEROOT = "http://sphere.sf.net/malis-test"; //NO TAILING SLASH

function start(){
  include_once("/home/groups/s/sp/sphere/htdocs/doctype.inc.php"); 
  echo "
  <html>
    <head>
      <title>sphere.sourceforge.net</title>
      <link rel=\"stylesheet\" type=\"text/css\" href=\"$__SITEROOT/utility.css\" />
    </head>
    <body>
      <table cellpadding=\"0\" cellspacing=\"0\" class=\"main\">
        <tr>
          <td class=\"topbar\">

            <table cellpadding=\"0\" cellspacing=\"0\" class=\"topmenu\">
              <tr>
                <td class=\"menuitems\">
                  <a class=\"headleft\"  href=\"$__SITEROOT/index.php\">news</a>
                  <a class=\"headleft\"  href=\"$__SITEROOT/index.php?U_ACT=articles\" style=\"text-decoration: line-through\">articles</a>
                  <a class=\"headleft\"  href=\"$__SITEROOT/dev.php?U_ACT=current\">download</a>
                  <a class=\"headleft\"  href=\"$__SITEROOT/dev.php?U_ACT=log\" style=\"text-decoration: line-through\">dev log</a>
                  <a class=\"headleft\"  style=\"border-bottom: none\" href=\"http://aegisknight.org/forum\">forums *</a>
                </td>

                <td class=\"menulogo\">
                  <img src=\"$__SITEROOT/images/title.png\" alt=\"sphere.sourceforge.net logo\" />
                  <form id=\"login\" method=\"post\" action=\"$__SITEROOT/account.php?U_ACT=login\">
                    <div>
                      <input type=\"text\" name=\"login_name\" size=\"8\" />
                      <a href=\"javascript:document.getElementById('login').submit()\"> login</a> or <a href=\"$__SITEROOT/account.php?U_ACT=register\">register</a> (<a href=\"http://sphere.sourceforge.net/malis-test/account.php?U_ACT=forgot\">forgot password?</a>)
                    </div>
                  </form>
                </td>
                
                <td class=\"menuitems\">
                  <a class=\"headright\" href=\"$__SITEROOT/projects.php?U_ACT=list\" style=\"text-decoration: line-through\">projects</a>
                  <a class=\"headright\" href=\"$__SITEROOT/dev.php?U_ACT=about\">about sphere</a>
                  <a class=\"headright\" href=\"$__SITEROOT/links.php\">links</a>
                  <a class=\"headright\" href=\"$__SITEROOT/index.php?U_ACT=map\">site map</a>
                  <a class=\"headright\" style=\"border-bottom: none\" href=\"$__SITEROOT/contact.php\">contact</a>
                </td>
              </tr>
            </table>

          </td>
        </tr>
      <tr>
        <td class=\"body\">
          currently under development. line-through means the page is not available yet. (* the forums are the old forums on <a href=\"http://aegisknight.org\">aegisknight.org</a> until the forum code for sphere.sf.net is completed.)
          <table class=\"bodybody\">
            <tr>
              <td class=\"bodybody\">";
}



  
function side_bar_item( $_TITLE, $_BODY ){
  echo "
                <table class=\"sidebaritem\">
                  <tr>
                    <td class=\"sidebaritemhead\">
                      $_TITLE
                    </td>
                  </tr>
                  <tr>
                    <td class=\"sidebaritembody\">
                      $_BODY
                    </td>
                  </tr>
                </table>";
}

function conclude(){
  global $__DATABASE;
  echo '
              </td>
              <td class="sidebar">';
              
  $TEMPBODY = "";
  mconnect();
  $QUERY = "SELECT * FROM NEWS_ENTRIES
    WHERE PARENT = 0
    ORDER BY ID DESC";
  $RESULT = mysql_db_query( $__DATABASE, $QUERY );

  while( $R = mysql_fetch_array( $RESULT ) ){
    $RID = $R['ID'];
    $RTOPIC = $R['TOPIC'];
    $TEMPBODY .= "<a href=\"index.php#$RID\">$RTOPIC</a><br />\n";
  }
  side_bar_item( 'latest news', $TEMPBODY );
  side_bar_item( 'latest additions', '
    20.10.01 - Games - Some dumb game<br />
    19.10.01 - Articles - Some dumb article<br />
    <br />
    <div style="text-align: right; width: 100%">complete ...</div>' );
  side_bar_item( 'my account', "if you're logged in, here you'll be able to edit all sorts of things like which panels on the side you want, possibly what theme you want the site in, if you want to set up a project and what projects you are part of etc. more on this to come hopefully." );
  echo '
              </td>
            </tr>
          </table>
          </td>
        </tr>
      </table>
    </body>
  </html>';
}

function gen_item( $_HEAD, $_BODY ){
  echo "
    <table class=\"item\">
      <tr>
        <td class=\"itemhead\">
          $_HEAD
        </td>
      </tr>
      <tr>
        <td class=\"itembody\">
          $_BODY
          <br />
        </td>
      </tr>
    </table>";
}

function news_item( $_ID, $_TOPIC, $_DATE, $_AID, $_ICON, $_BODY ){
  $AEMAIL = GetCellFromUID( $_AID, 'EMAIL' );
  $AUTHOR = GetCellFromUID( $_AID, 'NAME' );
  if( $_ICON == 1 ) $_ICON = "announcement";
    echo "<a name=\"$_ID\" />
          <table class=\"item\">
            <tr>
              <td class=\"itemhead\">
                <img src=\"images/icons/$_ICON.png\" style=\"vertical-align: middle\" alt=\"[ $_ICON ]\" /> $_TOPIC (<a href=\"$__SITEROOT/index.php?U_ACT=reply&amp;rid=$_ID\">entire thread</a>)
              </td>
            </tr>
            <tr>
              <td class=\"itembody\">
                $_BODY
                <br />
                <br />
                <div class=\"itembodyauthor\"> - <a href=\"mailto:$AEMAIL\">$AUTHOR</a> ($_DATE) </div>
              </td>
            </tr>
          </table>";
}

function important_links(){
  echo '
    <br />
    <div class="linkbar">
      <a href="http://validator.w3.org/check/referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0!" height="31" width="88" /></a>
      <a href="http://jigsaw.w3.org/css-validator/check/referer"><img style="border:0;width:88px;height:31px" src="http://jigsaw.w3.org/css-validator/images/vcss" alt="Valid CSS!" /></a>
      <a href="http://sourceforge.net"><img src="http://sourceforge.net/sflogo.php?group_id=873&amp;type=1" alt="sourceforge logo" /></a>
      <br /><span class="smalltext">please note, if you have a user account and are logged in, that this page may not verify with css depending on your theme. in your user preferences, when changing themes, it will be noted if a theme is css valid or not. we encourage you to use <a href="http://w3.org/style/css/">correct css</a> when creating a stylehseet.<br />
      also, this site most likely has display problems in 4.0 browsers. Use a 6.0 browser and, preferrably, use a browser besides Microsoft Internet Explorer 6.0(-) for Windows because it does not allow for alpha blending in PNGs.</span><br />
      <a href="http://kmeleon.org">get k-meleon</a> | <a href="http://mozilla.org">get mozilla</a>
    </div>';
}

function mconnect(){ //What can I say? I'm lazy.
  global $__PASS_MYSQL;
  mysql_connect("mysql.sourceforge.net", "sphere", $__PASS_MYSQL) or die("could not connect to the mysql database. contact malis@wolf-phantom.net");
}

function GetCellFromUID( $_ID, $_CELL ){
  global $__DATABASE;
  mconnect();
  $QUERY = "SELECT * FROM ACCOUNTS
            WHERE ID=$_ID";
  $RESULT = mysql_db_query( $__DATABASE, $QUERY );
  while( $R = mysql_fetch_array( $RESULT ) ){
    $RETURN = $R[$_CELL];
  }
  return( $RETURN );
}

function GETUIDFromName( $_NAME ){
  global $__DATABASE;
  mconnect();
  $QUERY = "SELECT * FROM ACCOUNTS
            WHERE NAME = \"$_NAME\"";
  $RESULT = mysql_db_query( $__DATABASE, $QUERY );
  if( $R = mysql_fetch_array( $RESULT ) ){
    $RETURN = $R['ID'];
  }
  return( $RETURN );
}

?>