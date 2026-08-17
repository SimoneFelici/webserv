echo "Content-Type: text/html"
echo

cat <<EOF
<!DOCTYPE html>
<html>
<head>
    <title>CGI Bash</title>
</head>
<body>
    <h1>Ciao dal CGI Bash!</h1>
    <p>Data e ora: $(date)</p>
</body>
</html>
EOF
