islandora_reporter
==================

Lets one store and run SPARQL queries.
Requires Tuque API library.

Settings for a query we should all be familiar with:

Name:
Colection Query

Query:
PREFIX fedora_relations: <info:fedora/fedora-system:def/relations-external#>
PREFIX fedora_model: <info:fedora/fedora-system:def/model#>
SELECT $label $member_object
FROM <#ri>
WHERE {
{
$member_object fedora_relations:isMemberOf $PID
}
UNION
{
$member_object fedora_relations:isMemberOfCollection $PID
}
OPTIONAL
{
$member_object fedora_model:label $label
}
}

Return/Label Map:
{"member_object":"Members"}

Preg Replace/Label Map:
{"$PID":"Collection"}

Content Model/Label Map (displays as tab):
{"islandora:collectionCModel":"Collection Report"}

URL for all members of all collections (replace 1 with correct index):
islandora_reporter/report/1

URL for all members of islandora:root(serialzed, urlencoded preg variable):
islandora_reporter/report/1/a%3A1%3A%7Bi%3A0%3Ba%3A3%3A%7Bs%3A4%3A%22preg%22%3Bs%3A4%3A%22%24PID%22%3Bs%3A7%3A%22replace%22%3Bs%3A14%3A%22islandora%3Aroot%22%3Bs%3A6%3A%22is_URI%22%3Bb%3A1%3B%7D%7D

Programatic:
islandora_reporter_get_report($query['query_index'], $array(array('preg' => '$PID', 'replace' => $pid, 'is_URI' => TRUE)))
