<?php

class SphereUser {
  var $errors;
  
  var $username;
  var $password;
  var $email;
  var $firstname;
  var $lastname;
  var $birthdate;
  var $photo;
  var $website;
  var $notify;
  var $permissions;
  var $created;

  function SphereUser ($user_name) {
    global $DB;
    $this->errors = array();
    if (!$this->setUserName($user_name)) {
      $_user = $DB->query("SELECT * FROM users WHERE username = '{$this->username}'");
      if ($_user[0] == 1) {
        $_data = &$_user[1][0];
        $this->password = $_data->password;
        $this->email = $_data->email;
        $this->firstname = $_data->firstname;
        $this->lastname = $_data->lastname;
        $this->birthdate = $_data->birthdate;
        $this->photo = $_data->photo;
        $this->website = $_data->website;
        $this->permissions = $_data->permissions;
        $this->created = $_data->created;
      }
    } else {
      $this->permissions = 0;
      $this->notify = 0;
    }
  }

  function cleanup () {
    global $DB;
    $DB->cleanup();
  }

  function exists () {
    if ($this->password)
      return TRUE;
    else
      return FALSE;
  }

  function errors () {
	 $return = array();
	 foreach ($this->errors as $error) {
      if ($error)
	     $return[] = $error;
    }
    return $return;
  }

  function setUserName ($user_name) {
    if (!$user_name)
      $this->errors['username'] = "empty username";
    else if (strlen($user_name) > 32)
      $this->errors['username'] = "username must be shorter than 33 characters";
    else if (preg_match("/\s/", $user_name))
      $this->errors['username'] = "username cannot contain whitespace";
    else {
      $this->username = $user_name;
      $this->errors['username'] = NULL;
    }
    return $this->errors['username'];
  }

  function setPassword ($pass_word, $verify) {
    if (!$pass_word)
      $this->errors['password'] = "missing password";
    else if (strlen($pass_word) > 12)
      $this->errors['password'] = "password must be shorter than 13 characters";
    else if (preg_match("/\s/", $pass_word))
      $this->errors['password'] = "password cannot contain whitespace";
    else if ($pass_word != $verify)
      $this->errors['password'] = "passwords do not match";
    else {
      $this->password = crypt($pass_word, "jc");
      $this->errors['password'] = NULL;
    }
    return $this->errors['password'];
  }

  function setBirthdate ($year, $month, $day) {
    if ($year != NULL and $year < 1900 or $year > 2002)
      $this->errors['birthdate'] = "birthdate year is out of range (1900-2002)";
    else if ($month != NULL and $month < 1 or $month > 12)
      $this->errors['birthdate'] = "birthdate month is out of range (1-12)";
    else if ($day != NULL and $day < 1 or $day > 31)
      $this->errors['birthdate'] = "birthdate day is out of range (1-31)";
    else {
      $this->birthdate = "$year-$month-$day";
      $this->errors['birthdate'] = NULL;
    }
    return $this->errors['birthdate'];
  }

  function setEmail ($address) {
    if (strlen($address) > 255)
      $this->errors['email'] = "email address must be shorter than 256 characters";
    else if (preg_match("/\s/", $address))
      $this->errors['email'] = "email address cannot contain whitespace";
    else {
      $this->email = $address;
      $this->errors['email'] = NULL;
    }
    return $this->errors['email'];
  }

  function setName ($first, $last) {
    if (strlen($first) > 64)
      $this->errors['name'] = "first name must be shorter than 65 characters";
    else if (strlen($last) > 64)
      $this->errors['name'] = "last name must be shorter than 65 characters";
    else {
      $this->firstname = trim($first);
      $this->lastname = trim($last);
      $this->errors['name'] = NULL;
    }
    return $this->errors['name'];
  }

  function setPhoto ($photo_url) {
    if (strlen($photo_url) > 255)
      $this->errors['photo'] = "photo URL must be shorter than 255 characters";
    else if (preg_match("/\s/", $photo_url))
      $this->errors['photo'] = "photo URL cannot contain whitespace";
    else {
      $this->photo = $photo_url;
      $this->errors['photo'] = NULL;
    }
    return $this->errors['photo'];
  }

  function setWebsite ($website_url) {
    if (strlen($website_url) > 255)
      $this->errors['website'] = "website URL must be shorter than 256 characters";
    else if (preg_match("/\s/", $website_url))
      $this->errors['website'] = "website URL cannot contain whitespace";
    else {
      $this->website = $website_url;
      $this->errors['website'] = NULL;
    }
    return $this->errors['website'];
  }

  function addPermission ($permission) {
    switch ($permission) {
      case "superuser":
        $this->permissions |= 1;
        break;
      case "reporter":
        $this->permissions |= 2;
        break;
      case "reviewer":
        $this->permissions |= 4;
        break;
      case "moderator":
        $this->permissions |= 8;
        break;
    }
  }

  function access ($permission) {
    if ($this->permissions & 1) /* superusers have access to everything */
      return TRUE;
    switch ($permission) {
      case "reporter":
        return ($this->permissions & 2);
      case "reviewer":
        return ($this->permissions & 4);
      case "moderator":
        return ($this->permissions & 8);
      default:
        return FALSE;
    }
  }

  function setNotification ($notification) {
    if ($notification)
      $this->notify = 1;
    else
      $this->notify = 0;
  }

  function commit () {
    global $DB;
    if (count($this->errors()) != 0)
      return "cannot commit user while errors exist";
    $DB->query("CREATE TABLE IF NOT EXISTS users (username VARCHAR(32), password VARCHAR(32), email VARCHAR(255), firstname VARCHAR(64), lastname VARCHAR(64), birthdate DATE, photo VARCHAR(255), website VARCHAR(255), notify TINYINT, permissions INT, created DATETIME)");
    $_result = $DB->query("INSERT INTO users VALUES ('{$this->username}', '{$this->password}', '{$this->email}', '{$this->firstname}', '{$this->lastname}', '{$this->birthdate}', '{$this->photo}', '{$this->website}', '{$this->notify}', '{$this->permissions}', NOW())");
    if ($_result[0] == -1)
      return $_result[1];
    else
      return NULL;
  }

  /* $password is not encrypted */
  function validate ($pass_word) {
    return ($this->password == crypt($pass_word, "jc"));
  }

  /* $password is encrypted */
  function verify ($pass_word) {
    return ($this->password == $pass_word);
  }
}

?>
