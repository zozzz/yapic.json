// http://www.json-generator.com/
[
  '{{repeat(50)}}',
  {
    _id: '{{objectId()}}',
    index: '{{index()}}',
    guid: '{{guid()}}',
    isActive: '{{bool()}}',
    balance: '{{floating(1000, 4000, 2, "$0,0.00")}}',
    picture: 'http://placehold.it/32x32',
    age: '{{integer(20, 40)}}',
    eyeColor: '{{random("blue", "brown", "green")}}',
    name: '{{firstName()}} {{surname()}}',
    gender: '{{gender()}}',
    company: '{{company().toUpperCase()}}',
    email: '{{email()}}',
    phone: '+1 {{phone()}}',
    address: '{{integer(100, 999)}} {{street()}}, {{city()}}, {{state()}}, {{integer(100, 10000)}}',
    about: '{{lorem(1, "paragraphs")}}',
    registered: '{{date(new Date(2014, 0, 1), new Date(), "YYYY-MM-ddThh:mm:ss Z")}}',
    latitude: '{{floating(-90.000001, 90)}}',
    longitude: '{{floating(-180.000001, 180)}}',
    tags: [
      '{{repeat(7)}}',
      '{{lorem(1, "words")}}'
    ],
    friends: [
      '{{repeat(3)}}',
      {
        id: '{{index()}}',
        name: '{{firstName()}} {{surname()}}',
        registered: '{{date(new Date(2014, 0, 1), new Date(), "YYYY-MM-ddThh:mm:ss Z")}}',
        relation: {
          likes: '{{integer(20, 40)}}'
        },
        avatar: 'http://placehold.it/_avatar/{{guid()}}.jpg'
      }
    ],
    greeting: function (tags) {
      return 'Hello, ' + this.name + '! You have ' + tags.integer(1, 10) + ' unread messages.';
    },
    favoriteFruit: function (tags) {
      var fruits = ['apple', 'banana', 'strawberry'];
      return fruits[tags.integer(0, fruits.length - 1)];
    },
    unicode: function (tags) {
      var chCount = tags.integer(1, 10);
      var res = {};
      var chars = [
        "A", "Á", "B", "C", "D", "E", "É", "F", "G", "H", "I", "Í", "J", "K", "L", "M", "N", "O", "Ó", "Ö", "Ő", "P",
        "Q", "R", "S", "T", "U", "Ú", "Ü", "Ű", "V", "W", "X", "Y", "Z",
        "अ", "आ", "क्", "ख्", "ग्", "घ्", "ङ्", "ह्", "तालु", "इ", "ई", "च्", "छ्", "ज्", "झ्", "ञ्", "य्", "श्",
        "मूर्धा", "ऋ", "ॠ", "ट्", "ठ्", "ड्", "ढ्", "ण्", "र्", "ष्",
        "Б", "В", "Г", "Д", "Е", "Ё", "Ж", "З", "И", "Й", "К", "Л", "М", "Н", "О", "П", "Р", "С", "Т", "У", "Ф", "Х", "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я"
      ];

      function rnd(chCount) {
        var str = "";
        for (var i = 0; i < chCount; i++) {
          str += chars[tags.integer(0, chars.length)];
        }
        return str;
      }

      for (var i = 0, l = tags.integer(5, 10); i < l; i++) {
        res[rnd(tags.integer(5, 10))] = rnd(tags.integer(20, 200));
      }

      return res;
    }
  }
]